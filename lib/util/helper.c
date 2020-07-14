#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mbedtls/base64.h>
#include <mbedtls/ecdh.h>

#include "color.h"
#include "helper.h"

size_t helper_base64_encode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;

    mbedtls_base64_encode(
        (u_char *)dst,
        dst_len - 1,
        &written,
        (const u_char *)src,
        src_len);
    dst[written] = 0;

    return written;
}

size_t helper_base64_decode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;

    mbedtls_base64_decode(
        (u_char *)dst,
        dst_len - 1,
        &written,
        (const u_char *)src,
        src_len);
    dst[written] = 0;

    return written;
}

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
