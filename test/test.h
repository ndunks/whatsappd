#pragma once
#include <malloc.h>
#include <string.h>

#include <helper.h>

#define EQUAL(expression, val)                       \
    do                                               \
        if ((ret_val = (expression)) != val)         \
        {                                            \
            warn(#expression " return %d", ret_val); \
            return ret_val;                          \
        }                                            \
        else                                         \
            ret_val = 0;                             \
    while (0)

#define ZERO(expression) EQUAL(expression, 0)
#define FALSY(expression) EQUAL(expression, 0)
#define TRUTHY(expression) EQUAL(expression, 1)

int ret_val;

extern int test_setup();
extern int test_main();
extern int test_cleanup();
