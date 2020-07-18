#include <wss.h>
#include "test.h"
int wss_connected = 0;
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
    char *msg = "HELLO WORLD EXAMPLES", *reply;
    int msg_len = strlen(msg);
    //ZERO(wss_connect("echo.websocket.org", NULL, "/"));
    ZERO(wss_connect("localhost", "8433", "/"));
    wss_connected = 1;

    EQUAL(wss_send_text(msg, msg_len), msg_len);
    reply = wss_read();
    info("REPLY: %s", reply);
    ZERO(strcmp(msg, reply));
    ZERO(strncmp(msg, reply, msg_len));
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
    if (wss_connected)
    {
        ZERO(wss_disconnect());
    }
    crypto_free();
    return 0;
}
