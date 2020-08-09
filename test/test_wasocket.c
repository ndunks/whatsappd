#include "crypto.h"
#include "wss.h"
#include "wasocket.h"

#include "test.h"

int test_tags()
{
    char *tag;
    int counter;

    tag = wasocket_short_tag();
    info("short_tag: %s", tag);
    TRUTHY(strlen(tag) > 0);
    TRUTHY(strlen(tag) <= 8);

    counter = atoi(strrchr(tag, '-') + 1);
    TRUTHY(counter == 0);

    tag = wasocket_short_tag();
    TRUTHY(strlen(tag) > 0);
    TRUTHY(strlen(tag) <= 8);
    counter = atoi(strrchr(tag, '-') + 1);
    TRUTHY(counter == 1);

    // long tag  1595220140
    tag = wasocket_tag();
    info("long_tag: %s", tag);
    TRUTHY(strlen(tag) > 0);
    EQUAL(strlen(tag), 15);
    counter = atoi(strrchr(tag, '-') + 1);
    TRUTHY(counter == 2);

    return 0;
}

int test_send_init()
{
    ssize_t reply_len;
    int sent, len;
    char buf[255], client_id[75], *reply, *reply_tag, *tag = "1818181,";

    ZERO(crypto_init());
    //ZERO(wss_connect(NULL, NULL, NULL));
    //ZERO(wss_connect("localhost", "8433", "/"));
    ZERO(wss_connect("echo.websocket.org", NULL, "/"));

    crypto_random(buf, 16);

    crypto_base64_encode(client_id, 75, buf, CFG_CLIENT_ID_LEN);
    info("ClientID: %s", client_id);

    len = sprintf(buf, "[\"admin\",\"init\","
                       "[2,2019,6],[\"whatsappd\",\"github.com/ndunks\",\"%s\"],"
                       "\"%s\",true]",
                  (sizeof(void *) == 4) ? "x86" : "x86_64",
                  client_id);

    sent = wasocket_send_text(buf, len, NULL);
    TRUTHY(sent >= len);
    ZERO(wasocket_read(&reply, &reply_tag, &reply_len));
    TRUTHY(reply_len == len);
    ZERO(strncmp(buf, reply, len > reply_len ? len : reply_len));

    accent("** with custom tag: %s", tag);
    sent = wasocket_send_text(buf, len, tag);
    TRUTHY(sent >= len);
    ZERO(wasocket_read(&reply, &reply_tag, &reply_len));
    info("%ld, tag: %s, data: %s", reply_len, reply_tag, reply);
    TRUTHY(reply_len == len);
    ZERO(strncmp(buf, reply, len > reply_len ? len : reply_len));

    // Comma is replaced by null in reply_tag, so exclude it
    ZERO(strncmp(tag, reply_tag, strlen(tag) - 1));

    wss_disconnect();
    crypto_free();
    return 0;
}

int test_main()
{
    return test_tags() || test_send_init();
}

int test_setup()
{
    wasocket_setup();
    return 0;
}

int test_cleanup()
{
    return 0;
}
