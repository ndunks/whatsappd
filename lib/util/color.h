#pragma once
#include <stdio.h>

#define COL_RED "\x1b[31m"
#define COL_GREEN "\x1b[32m"
#define COL_YELL "\x1b[33m"
#define COL_BLUE "\x1b[34m"
#define COL_MAG "\x1b[35m"
#define COL_CYN "\x1b[36m"
#define COL_NORM "\x1b[0m"

#define debug(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define info(fmt, ...) debug(COL_CYN fmt COL_NORM, ##__VA_ARGS__)
#define ok(fmt, ...) debug(COL_GREEN fmt COL_NORM, ##__VA_ARGS__)
#define warn(fmt, ...) debug(COL_YELL fmt COL_NORM, ##__VA_ARGS__)
#define err(fmt, ...) debug(COL_RED fmt COL_NORM, ##__VA_ARGS__)
#define accent(fmt, ...) debug(COL_MAG fmt COL_NORM, ##__VA_ARGS__)
#define hexdump(buf, size)             \
    do                                 \
    {                                  \
        for (int i = 0; i < size; i++) \
        {                              \
            printf("%02x", buf[i]);    \
        }                              \
        printf("\n");                  \
    } while (0)