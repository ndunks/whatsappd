#pragma once
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "color.h"

#define hexdump(buf, size)                            \
    do                                                \
    {                                                 \
        for (int i = 0; i < size; i++)                \
        {                                             \
            printf("%02x ", (unsigned char)(buf)[i]); \
        }                                             \
        printf("\n");                                 \
    } while (0)

#define TRY(expression)                         \
    if ((CATCH_RET = (expression)) != 0)        \
    {                                           \
        err(#expression ": RET %d", CATCH_RET); \
        goto CATCH;                             \
    }

#define CHECK(expression)                \
    if ((CATCH_RET = (expression)) != 0) \
        return CATCH_RET;

extern int CATCH_RET;

#define die(msg)  \
    do            \
    {             \
        err(msg); \
        exit(1);  \
    } while (0)

uint64_t helper_jid_to_num(const char *buf);
int helper_qrcode_show(const char *src);
