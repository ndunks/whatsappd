#include <crypto.h>
#include <wss.h>
#include <wasocket.h>

#include "test.h"

int test_tags()
{
    char *tag;
    int counter;
    wasocket_setup();

    tag = wasocket_short_tag();
    info("short_tag: %s", tag);
    TRUTHY(strlen(tag) > 0);
    TRUTHY(strlen(tag) <= 7);

    counter = atoi(strrchr(tag, '-') + 1);
    TRUTHY(counter == 0);

    tag = wasocket_short_tag();
    TRUTHY(strlen(tag) > 0);
    TRUTHY(strlen(tag) <= 7);
    counter = atoi(strrchr(tag, '-') + 1);
    TRUTHY(counter == 1);

    // long tag  1595220140
    tag = wasocket_tag();
    info("long_tag: %s", tag);
    TRUTHY(strlen(tag) > 0);
    EQUAL(strlen(tag), 14);
    counter = atoi(strrchr(tag, '-') + 1);
    TRUTHY(counter == 2);

    return 0;
}

int test_send_init()
{
    ZERO(crypto_init());
    ZERO(wss_connect(NULL, NULL, NULL));
    wss_disconnect();
    crypto_free();
    return 0;
}

int test_main()
{
    return test_tags();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
