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
    size_t recv_len;
    char *msg = "HELLO WORLD EXAMPLES", *reply;
    int msg_len = strlen(msg);
    EQUAL(wss_send_text(msg, msg_len), msg_len);
    reply = wss_read(&recv_len);
    info("REPLY: %s", reply);
    ZERO(strcmp(msg, reply));
    ZERO(strncmp(msg, reply, msg_len));
    return 0;
}

int test_send(size_t msg_len, char ch)
{
    size_t recv_len = 0;
    char msg[msg_len], *reply;
    memset(msg, ch, msg_len);
    TRUTHY(wss_send_binary(msg, msg_len) == msg_len);
    reply = wss_read(&recv_len);
    info("%lu vs %lu", recv_len, msg_len);
    TRUTHY(recv_len == msg_len);
    //fwrite(reply, 1, recv_len, stderr);
    ZERO(memcmp(msg, reply, msg_len));
    return 0;
}

int test_main()
{
    //return test_mask() || test_size_7_bit() || test_size_16_bit();
    //return test_send(0x100, 'f'); // 16 bit;
    //return test_send(0x1000, 'x'); // 16 bit;
    return test_send(0x10000, 'x'); // 64 bit;
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
