#include <wss.h>
#include "test.h"

int test_mask()
{
    uint32_t a, b, c;

    TRUTHY(a = wss_mask());
    TRUTHY(b = wss_mask());
    TRUTHY(c = wss_mask());

    FALSY(a == b);
    FALSY(a == c);
    FALSY(b == c);
    TRUTHY(c = wss_mask());
    return 0;
}

int test_connection()
{
    char *msg = "HELLO WORLD EXAMPLES";
    int msg_len = strlen(msg);
    ZERO(wss_connect("echo.websocket.org", NULL, "/"));

    EQUAL(wss_send_text(msg, msg_len), msg_len);
    wss_read();
    ZERO(wss_disconnect());
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
