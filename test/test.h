#pragma once

#include <whatsappd.h>

#define ASSERT(expression)                          \
    do                                              \
        if ((ret_val = expression) != 0)            \
        {                                           \
            warn(#expression " return %d", ret_val); \
            return ret_val;                         \
        }                                           \
    while (0)

int ret_val;

extern int test_setup();
extern void test_main();
extern int test_cleanup();
