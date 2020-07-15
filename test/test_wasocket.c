#include <wasocket.h>

#include "test.h"

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
    ZERO(crypto_init());
    return 0;
}

int test_cleanup()
{
    crypto_free();
    return 0;
}
