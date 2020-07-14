#include <wasocket.h>

#include "test.h"

static char *buf;

int test_connection()
{
    ZERO(wasocket_connect());
    ZERO(wasocket_disconnect());
    return 0;
}

int test_main()
{
    return test_connection();
}

int test_setup()
{
    buf = malloc(255);
    ZERO(crypto_init());
    return 0;
}

int test_cleanup()
{
    free(buf);
    ZERO(crypto_free());
    return 0;
}
