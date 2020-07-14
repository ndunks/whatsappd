#include <string.h>

#include "color.h"
#include "helper.h"

static size_t write_config_buf(char *str, char *buf)
{
    size_t size = strlen(str);

    if (size > 255)
        die("write config str to large");

    buf[0] = size;
    if (size > 0)
    {
        memcpy(&buf[1], str, size);
    }

    return size + 1;
}

static size_t read_config_buf(char **str, char *buf)
{
    size_t size = (u_int8_t)buf[0];

    if (size > 255)
        die("read config str to large");

    if (size > 0)
    {
        *str = malloc(size + 1);
        memcpy(*str, &buf[1], size);
        (*str)[size] = 0;
    }
    else
        *str = NULL;

    return size + 1;
}
