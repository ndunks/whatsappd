#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <wsclient.h>

#include "test.h"

static char *buf;
typedef struct
{
    pthread_t wsthread;
    wsclient_t *wsclient;
    bool is_conn;
    wsclient_message_t recv_msg;
} wsclient_test_t;
static wsclient_test_t wstest;

static int onopen(wsclient_t *client, wsclient_test_t *wstest)
{
    printf("onopen()\n");
    wstest->is_conn = true;
    client->send_text(client, "1594275999.--0,[\"admin\",\"init\",[2,2025,6],[\"Linux\",\"Chrome\",\"x86_64\"],\"M8j29DnY1i5rZGTPS0mFKw==\",true]");
    return 0;
}

static int onmessage(wsclient_t *client, wsclient_message_t *msg, wsclient_test_t *wstest)
{
    printf("onmessage(opcode:%04x, len:%zu)\n", msg->opcode, msg->payload_len);
    printf("\t");
    fprintf(stdout, msg->payload, msg->payload_len);
    // for (size_t i = 0; i < msg->payload_len; i++)
    // {
    //     printf("%02x ", msg->payload[i]);
    // }
    printf("\n");
    wstest->recv_msg.opcode = msg->opcode;
    wstest->recv_msg.payload_len = msg->payload_len;
    if (wstest->recv_msg.payload != NULL)
    {
        free(wstest->recv_msg.payload);
    }
    wstest->recv_msg.payload = (uint8_t *)malloc(msg->payload_len);
    memcpy(wstest->recv_msg.payload, msg->payload, msg->payload_len);
    return 0;
}

static int onerror(wsclient_t *client, wsclient_error_t *err, wsclient_test_t *wstest)
{
    printf("onerror()\n");
    return 0;
}

static int onclose(wsclient_t *client, wsclient_test_t *wstest)
{
    printf("onclose()\n");
    wstest->is_conn = false;
    return 0;
}
wsclient_config_t config;
int test_connect()
{

    ASSERT(wstest.wsclient == NULL);
    wstest.wsclient->run(wstest.wsclient);

    return 0;
}

int test_disconnect()
{
    ASSERT(0);
    return 0;
}

int test_main()
{
    return test_connect() || test_disconnect();
}

int test_setup()
{
    buf = malloc(255);
    wstest.is_conn = false;
    wstest.wsthread = NULL;
    wstest.wsclient = NULL;
    // config.uri = "wss://echo.websocket.org";
    // config.extra_header = "Origin: https://echo.websocket.org";
    config.uri = "wss://web.whatsapp.com/ws";
    config.extra_header = "Origin: https://web.whatsapp.com";
    config.heart_beat_interval = 10000;
    config.open_cb = (on_open_cb)onopen;
    config.message_cb = (on_message_cb)onmessage;
    config.error_cb = (on_error_cb)onerror;
    config.close_cb = (on_close_cb)onclose;
    config.cbdata = &wstest;
    wstest.wsclient = wsclient_new(&config);
    return 0;
}

int test_cleanup()
{
    //wstest.wsclient->shutdown(wstest.wsclient);
    free(buf);
    return 0;
}
