#include "wa.h"

#include "test.h"

int test_main()
{
    char buf[21];
    wa_create_msg_id(buf);

    TRUTHY(strlen(buf) == 20);

    wa_sanitize_jid_long(buf, "6285726767672");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    wa_sanitize_jid_long(buf, "62999999999@xxxx");
    ZERO(strcmp(buf, "62999999999@s.whatsapp.net"));

    wa_sanitize_jid_long(buf, "6285726767672@c.us");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    wa_sanitize_jid_long(buf, "6285726767672@@x");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    wa_sanitize_jid_short(buf, "6285726767672");
    ZERO(strcmp(buf, "6285726767672@c.us"));

    wa_sanitize_jid_short(buf, "62999999999@xxxx");
    ZERO(strcmp(buf, "62999999999@c.us"));

    wa_sanitize_jid_short(buf, "6285726767672@c.us");
    ZERO(strcmp(buf, "6285726767672@c.us"));

    wa_sanitize_jid_short(buf, "6285726767672@@x");
    ZERO(strcmp(buf, "6285726767672@c.us"));

    return 0;
}

int test_setup()
{
    crypto_init();
    return 0;
}

int test_cleanup()
{
    crypto_free();
    return 0;
}
