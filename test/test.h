#pragma once
#include <malloc.h>
#include <string.h>
#include <color.h>

#define COMPARE(expression, op, val)             \
    if ((ret_val = (expression)) op val)         \
    {                                            \
        debug(                                   \
            "%s%s %s%s %s%d %sbut %d%s",       \
            COL_YELL, #expression, COL_MAG, #op, \
            COL_CYN, val,                       \
            COL_RED, ret_val, COL_NORM);         \
        return 1;                                \
    }

#define EQUAL(expression, val) COMPARE(expression, !=, val)
#define NOTEQUAL(expression, val) COMPARE(expression, ==, val)

#define ZERO(expression) EQUAL(expression, 0)
#define FALSY(expression) EQUAL(expression, 0)
#define TRUTHY(expression) NOTEQUAL(expression, 0)

extern int ret_val;
extern int test_setup();
extern int test_main();
extern int test_cleanup();
