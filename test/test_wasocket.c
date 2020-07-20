#include <crypto.h>
#include <wss.h>
#include <wasocket.h>

#include "test.h"

int test_tags()
{
    char *short_tag;
    int counter;
    wasocket_setup();

    short_tag = wasocket_short_tag();
    info("short_tag: %s", short_tag);
    TRUTHY(strlen(short_tag) > 0);
    TRUTHY(strlen(short_tag) <= 7);

    counter = atoi(strrchr(short_tag, '-')+1);
    TRUTHY(counter == 0);

    short_tag = wasocket_short_tag();
    TRUTHY(strlen(short_tag) > 0);
    TRUTHY(strlen(short_tag) <= 7);
    counter = atoi(strrchr(short_tag, '-')+1);
    TRUTHY(counter == 1);
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
