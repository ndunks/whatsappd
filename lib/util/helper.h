#pragma once
#include <string.h>
#include <stdlib.h>
#include "color.h"

#define die(msg)          \
    do                    \
    {                     \
        printf(msg "\n"); \
        exit(1);          \
    } while (0)


int helper_qrcode_show(const char *src);
