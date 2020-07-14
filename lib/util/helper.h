#pragma once
#include <stdlib.h>

#define die(msg)          \
    do                    \
    {                     \
        printf(msg "\n"); \
        exit(1);          \
    } while (0)

