#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

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
    wss_write_chunk(tag, 0, tag_len, &mask);
    wss_write_chunk(data, tag_len, len + tag_len, &mask);
    info("---\n%s%s\n---", tag, data);
    return wss_send();
}

// Read and remove tags
int wasocket_read(char **data, char **tag, ssize_t *data_size)
{
    wss_read(NULL);

    *data = strchr(wss.rx, ',');
    if (*data == NULL)
    {
        err("No tag in message!");
        return 1;
    }
    *data[0] = 0;
    (*data)++;
    *tag = wss.rx;

    *data_size = wss.rx_len - (*data - *tag);
    return 0;
}

void *wasocket_thread()
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
        info("Got %ld bytes", size);

    } while (1);
    warn("wasocket thread exit");
    return NULL;
}