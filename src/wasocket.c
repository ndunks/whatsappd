#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <helper.h>
#include <wss.h>

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
int wasocket_send_text(char *data, uint len, char *tag)
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
    wss_frame(WS_OPCODE_TEXT, tag_len + len, mask);
    wss_write_chunk((uint8_t *)tag, 0, tag_len, &mask);
    wss_write_chunk((uint8_t *)data, tag_len, len + tag_len, &mask);
    info("---\n%s%s\n---", tag, data);
    return wss_send();
}

// Read and remove tags
int wasocket_read(char **data, char **tag, ssize_t *data_size)
{
    char *buf;
    int ret;
    size_t buf_len, decrypted_len;

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
        WSS_NEED_BUF(*data_size);
        buf = wss.buf + wss.buf_len;
        memset(buf, 0, *data_size);
        wss.buf_len += *data_size;
        decrypted_len = *data_size;

        info("TAG (%ld): %s: datasz: %lu", strlen(*tag), *tag, *data_size);
        hexdump(*data, *data_size);
        accent("-----------");
        ret = crypto_decrypt_hmac(data, &decrypted_len, buf);
        wss.buf_len -= (*data_size);
        if (ret)
        {
            return ret;
        };
        info("Size dec %lu vs %lu", *data_size, decrypted_len);
        (*data_size) = decrypted_len;
    }

    info("TAG (%ld): %s", *data - *tag - 1, *tag);
    if (*data_size < 1024)
    {
        // if (wss_frame_rx.opcode == WS_OPCODE_TEXT)
        // {
        //accent("%s\n-----------", *data);
        // }
        // else
        // {
        //     hexdump(*data, *data_size);
        //     accent("-----------");
        // }
    }

    return 0;
}

int wasocket_read_all(uint32_t timeout_ms)
{
    int ret = 1;
    ssize_t size;
    char *msg, *tag;
    accent("-------\nwasocket_read_all");
    do
    {
        ret = ssl_check_read((ret > 1) ? 100 : timeout_ms);
        info("ssl_check_read: %d", ret);
        if (ret > 0)
        {
            info("%p == %ld", &size, size);
            if (wasocket_read(&msg, &tag, &size))
                return 1;

            info("%p == %ld", &size, size);
            info("Got %ld bytes", size);
            if (size < 256)
            {
                hexdump(msg, size);
                warn("-------------------");
                fwrite(msg, 1, size, stderr);
                warn("\n-------------------");
            }
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