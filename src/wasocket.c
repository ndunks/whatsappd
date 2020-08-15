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
time_t wasocket_last_sent = 0;

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
    time(&wasocket_last_sent);

#ifdef DEBUG
    warn("SEND: %s (%s) ", tag, opcode == WS_OPCODE_BINARY ? "BIN" : "TXT");
    if (len < 90)
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

// Read and split tag
int wasocket_read(char **data, char **tag, ssize_t *data_size)
{
    char *decrypted;
    size_t encrypted_len;

    if (wss_read(NULL) == NULL)
        return 1;

    *tag = wss.rx;
    // timeskew
    if (**tag == '!')
    {
        *data_size = 0;
        *data = NULL;
        return 0;
    }

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
        decrypted = wss.buf + wss.buf_len;
        wss.buf_len += encrypted_len;

        CHECK(crypto_decrypt_hmac(*data, encrypted_len, decrypted, (size_t *)data_size));

#ifdef SAVE_MSG
        util_save_msg(*tag, decrypted, *data_size, *data, encrypted_len, true);
#endif
        mempcpy(*data, decrypted, *data_size);
        wss.buf_len -= encrypted_len;
    }
#ifdef SAVE_MSG
    else
    {
        util_save_msg(*tag, *data, *data_size, NULL, 0, false);
    }
#endif

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
    accent("wasocket_read_reply %s", req_tag);
    do
    {
        ret = wss_ssl_check_read(900);
        if (ret > 0)
        {
            if (wasocket_read(&msg, &tag, &size) != 0)
                return NULL;
            if (size > 0 && strncmp(tag, req_tag, tag_len) == 0)
                return msg;
        }
    } while (ret > 0);
    return NULL;
}

// Read command tag reply s12,...
size_t wasocket_read_command_reply(const char *type, char *buf, size_t buf_len)
{
    ssize_t size;
    char *msg, *tag;
    int ret = 1;
    size_t tag_len, num_len, len;
    len = strlen(type);

    accent("wasocket_read_command_reply");
    do
    {
        ret = wss_ssl_check_read(1000);
        if (ret <= 0)
            return 0;

        if (wasocket_read(&msg, &tag, &size) != 0)
            return 0;

        if (*tag != 's' || size <= 0)
            continue;

        tag_len = strlen(tag);
        num_len = strspn(tag + 1, "1234567890");
        if (tag_len != num_len + 1)
            continue;

        if (msg[0] == '[' && msg[1] == '"' && strncmp(&msg[2], type, len) == 0)
        {
            if (size + 1 > buf_len)
            {
                warn("Truncated command reply %lu > %lu", size, buf_len);
                size = buf_len;
            }
            memcpy(buf, msg, size);
            buf[size - 1] = 0;
            return size;
        }

    } while (1);
}

int wasocket_read_all(uint32_t timeout_ms)
{
    int ret = 1;
    ssize_t size;
    char *msg, *tag;
    accent("wasocket_read_all");
    do
    {
        ret = wss_ssl_check_read(timeout_ms);
        if (ret > 0)
        {
            if (wasocket_read(&msg, &tag, &size) != 0)
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
        if (wasocket_read(&msg, &tag, &size) != 0)
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