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

int test_size_7_bit()
{
    char *msg = "HELLO WORLD EXAMPLES", *reply;
    int msg_len = strlen(msg);
    EQUAL(wss_send_text(msg, msg_len), msg_len);
    reply = wss_read();
    info("REPLY: %s", reply);
    ZERO(strcmp(msg, reply));
    ZERO(strncmp(msg, reply, msg_len));
    return 0;
}

int test_size_16_bit()
{
    uint16_t msg_len = 0xfff;
    char msg[msg_len], *reply;
    memset(msg, 'f', 0xfff);
    EQUAL(wss_send_binary(msg, msg_len), msg_len);
    reply = wss_read();
    ZERO(memcmp(msg, reply, msg_len));
    return 0;
}

int test_main()
{
    //return test_mask() || test_size_7_bit() || test_size_16_bit();
    return test_size_16_bit();
}

int test_setup()
{
    ZERO(crypto_init());
    //ZERO(wss_connect("echo.websocket.org", NULL, "/"));
    ZERO(wss_connect("localhost", "8433", "/"));
    return 0;
}

int test_cleanup()
{
    ZERO(wss_disconnect());
    crypto_free();
    return 0;
}
