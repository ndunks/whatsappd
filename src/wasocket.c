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
static pthread_t wasocket_thread;
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
    if (wss_read(NULL) == NULL)
        return 1;

    *data = strchr(wss.rx, ',');
    if (*data == NULL)
    {
        err("No tag in message!");
        return 1;
    }
    *data[0] = 0;
    (*data)++;
    *tag = wss.rx;

    info("TAG LEN: %ld", *data - *tag);

    *data_size = wss.rx_len - (*data - *tag);
    return 0;
}

int wasocket_read_all(uint32_t timeout_ms)
{
    int ret = 1;
    ssize_t size;
    char *msg, *tag;

    do
    {
        ret = ssl_check_read((ret > 1) ? 100 : timeout_ms);
        info("ssl_check_read: %d", ret);
        if (ret > 0)
        {
            if (wasocket_read(&msg, &tag, &size))
            {
                err("wasocket_thread: read fail");
                return 1;
            }

            info("Got %ld bytes\n%s", size, tag);
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
        {
            err("wasocket_thread: read fail");
            break;
        }

        info("Got %ld bytes\n%s", size, tag);
        if (size < 256)
        {
            hexdump(msg, size);
            warn("-------------------");
            fwrite(msg, 1, size, stderr);
            warn("\n-------------------");
        }

    } while (size >= 0);

    warn("wasocket thread exit");
    return NULL;
}

int wasocket_start()
{
    return pthread_create(&wasocket_thread, NULL, wasocket_run, NULL);
}

int wasocket_stop()
{
    pthread_cancel(wasocket_thread);
    pthread_join(wasocket_thread, NULL);
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