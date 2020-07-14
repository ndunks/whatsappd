#pragma once
#include <stdlib.h>

#define die(msg)          \
    do                    \
    {                     \
        printf(msg "\n"); \
        exit(1);          \
    } while (0)

size_t helper_base64_encode(char *dst, size_t dst_len, const char *src, size_t src_len);
size_t helper_base64_decode(char *dst, size_t dst_len, const char *src, size_t src_len);
