#include <wasocket.h>

#include "test.h"
int test_mask()
{
    uint32_t a, b, c, d;
    TRUTHY(a = wasocket_mask());
    TRUTHY(b = wasocket_mask());
    TRUTHY(c = wasocket_mask());
    TRUTHY(d = wasocket_mask());
    hexdump(((char *)&a), 4);
    hexdump(((char *)&b), 4);
    hexdump(((char *)&c), 4);
    hexdump(((char *)&d), 4);
    return 0;
}

int test_connection()
{
    ZERO(wasocket_connect());
    ZERO(wasocket_disconnect());
    return 0;
}

int test_main()
{
    return test_mask() || test_connection();
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
