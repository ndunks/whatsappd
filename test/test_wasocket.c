#include <wasocket.h>

#include "test.h"

static char *buf;

int test_connect()
{
    ZERO(wasocket_connect());
    return 0;
}

int test_disconnect()
{
    ZERO(wasocket_disconnect());
    return 0;
}

int test_main()
{
    return test_connect() || test_disconnect();
}

int test_setup()
{
    buf = malloc(255);
    return 0;
}

int test_cleanup()
{
    free(buf);
    return 0;
}
