#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

#include <helper.h>
#include <wss.h>

#include "wasocket.h"

static uint64_t msg_counter = 0;

char tag_buf[32] = {0},
     short_tag_base[5] = {0},
     tag_fmt[] = "%lu.--%lu",
     short_tag_fmt[] = "%s.--%lu";

void wasocket_setup()
{
    sprintf(short_tag_base, "%ld", (time(NULL) % 1000));
}

char *wasocket_short_tag()
{
    sprintf(tag_buf, short_tag_fmt, short_tag_base, msg_counter++);
    return tag_buf;
}

char *wasocket_tag()
{
    sprintf(tag_buf, tag_fmt, time(NULL), msg_counter++);
    return tag_buf;
}

int wasocket_send(char *data, uint len)
{
}