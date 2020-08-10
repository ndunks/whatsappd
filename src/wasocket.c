#include <malloc.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "wasocket.h"

static uint64_t msg_counter = 0;
static uint32_t mask;
static int tag_len = 0;
static char tag_buf[32] = {0},
            short_tag_base[5] = {0},
            tag_fmt[] = "%lu.--%lu,",
            short_tag_fmt[] = "%s.--%lu,";
static pthread_t wasocket_thread = 0;
static pthread_mutex_t wasocket_mutex = PTHREAD_MUTEX_INITIALIZER;

void wasocket_setup()
{
    sprintf(short_tag_base, "%ld", (time(NULL) % 1000));
}

char *wasocket_short_tag()
{
    tag_len = sprintf(tag_buf, short_tag_fmt, short_tag_base, msg_counter++);
    return tag_buf;
}

char *wasocket_tag()
{
    tag_len = sprintf(tag_buf, tag_fmt, time(NULL), msg_counter++);
    return tag_buf;
}

/* null custom_tag will auto create tag */
size_t wasocket_send(char *data, uint len, char *tag, enum WS_OPCODE opcode)
{
    if (tag == NULL)
    {
        tag = wasocket_tag();
    }
    else
    {
        tag_len = strlen(tag);
    }

    mask = wss_mask();
    wss_frame(opcode, tag_len + len, mask);
    wss_write_chunk((uint8_t *)tag, 0, tag_len, &mask);
    wss_write_chunk((uint8_t *)data, tag_len, len + tag_len, &mask);

#ifdef DEBUG
    warn("SEND: %s (%s) ", tag, opcode == WS_OPCODE_BINARY ? "BIN" : "TXT");
    if (len < 256)
    {
        if (opcode == WS_OPCODE_TEXT)
            fwrite(data, 1, len, stderr);
        else
            hexdump(data, len);
        warn("------- ");
    }
#endif
    return wss_send();
}

/* null custom_tag will auto create tag */
size_t wasocket_send_text(char *data, uint len, char *tag)
{
    return wasocket_send(data, len, tag, WS_OPCODE_TEXT);
}

size_t wasocket_send_binary(char *data, uint len, char *tag, BINARY_METRIC metric, uint8_t flag)
{
    char *data_ptr, *encrypted_ptr;
    size_t data_size = 0, encrypted_size, sent_size = 0, req_buffer;
    req_buffer = len + 64;
    WSS_NEED_BUF(req_buffer);
    data_ptr = &wss.buf[wss.buf_idx];
    wss.buf_idx += req_buffer;

    if (tag == NULL)
        tag = wasocket_short_tag();

    if (metric)
    {
        if (flag == 0)
            flag = EPHEMERAL_IGNORE;

        data_ptr[0] = metric;
        data_ptr[1] = flag;
        encrypted_ptr = data_ptr + 2;
        data_size += 2;
    }
    else
    {
        encrypted_ptr = data_ptr;
    }

    TRY(crypto_encrypt_hmac(data, len, encrypted_ptr, &encrypted_size));

    data_size += encrypted_size;

    sent_size = wasocket_send(data_ptr, data_size, tag, WS_OPCODE_BINARY);

CATCH:
    wss.buf_idx -= req_buffer;
    return sent_size;
}

// Read and remove tags
int wasocket_read(char **data, char **tag, ssize_t *data_size)
{
    char *ptr;
    size_t encrypted_len;

    if (wss_read(NULL) == NULL)
        return 1;

    *tag = wss.rx;
    *data = strchr(wss.rx, ',');
    if (*data == NULL)
    {
        err("No tag in message!");
        return 1;
    }

    **data = 0;
    (*data)++;
    *data_size = wss.rx_len - (*data - *tag);

    if (wss_frame_rx.opcode == WS_OPCODE_BINARY)
    {
        encrypted_len = *data_size;
        WSS_NEED_BUF(encrypted_len);
        ptr = wss.buf + wss.buf_len;
        wss.buf_len += encrypted_len;

        CHECK(crypto_decrypt_hmac(*data, encrypted_len, ptr, (size_t *)data_size));
        mempcpy(*data, ptr, *data_size);
        wss.buf_len -= encrypted_len;
    }

#ifdef DEBUG
    ok("READ (%s): %s %lu bytes", wss_frame_rx.opcode == WS_OPCODE_TEXT ? "TXT" : "BIN", *tag, *data_size);
    if (*data_size < 256)
    {
        if (wss_frame_rx.opcode == WS_OPCODE_TEXT)
        {
            accent("%s\n-----------", *data);
        }
        else
        {
            hexdump(*data, *data_size);
            accent("-----------");
        }
    }
#endif

    return 0;
}

// Read until specified tag received
char *wasocket_read_reply(char *req_tag)
{
    ssize_t size;
    char *msg, *tag;
    int ret = 1, tag_len;

    if (req_tag == NULL)
        req_tag = tag_buf;

    tag_len = strlen(req_tag) - 1;
    if (req_tag[tag_len - 1] == ',')
        tag_len -= 1;
    accent("-------\nwasocket_read_reply %s", req_tag);
    do
    {
        ret = wss_ssl_check_read(500);
        if (ret > 0)
        {
            if (wasocket_read(&msg, &tag, &size))
                return 1;
            if (size > 0 && strncmp(tag, req_tag, tag_len) == 0)
                return msg;
        }
    } while (ret > 0);
    return NULL;
}

int wasocket_read_all(uint32_t timeout_ms)
{
    int ret = 1;
    ssize_t size;
    char *msg, *tag;
    accent("-------\nwasocket_read_all");
    do
    {
        ret = wss_ssl_check_read(timeout_ms);
        info("ssl_check_read: %d", ret);
        if (ret > 0)
        {
            info("%p == %ld", &size, size);
            if (wasocket_read(&msg, &tag, &size))
                return 1;
        }
    } while (ret > 0);
    return 0;
}

static void *wasocket_run()
{
    ssize_t size;
    char *msg, *tag;

    do
    {
        if (wasocket_read(&msg, &tag, &size))
            break;

        info("Got %ld bytes\n%s", size, tag);
        if (size < 256)
        {
            hexdump(msg, size);
            warn("-------------------");
            fwrite(msg, 1, size, stderr);
            warn("\n-------------------");
        }

    } while (size > 0);

    warn("wasocket thread exit");
    return NULL;
}

int wasocket_start()
{
    return pthread_create(&wasocket_thread, NULL, wasocket_run, NULL);
}

int wasocket_stop()
{
    if (wasocket_thread)
    {
        pthread_cancel(wasocket_thread);
        pthread_join(wasocket_thread, NULL);
        wasocket_thread = 0;
    }
    return 0;
}

int wasocket_lock()
{
    return pthread_mutex_lock(&wasocket_mutex);
}

int wasocket_unlock()
{
    return pthread_mutex_unlock(&wasocket_mutex);
}