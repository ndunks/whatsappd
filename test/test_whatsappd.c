#include "whatsappd.h"

#include "test.h"
int test_functions()
{

    char buf[21];
    whatsappd_create_msg_id(buf);
    info("ID: %s", buf);
    TRUTHY(strlen(buf) == 20);

    whatsappd_sanitize_jid(buf, "6285726767672");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    whatsappd_sanitize_jid(buf, "62999999999@xxxx");
    ZERO(strcmp(buf, "62999999999@s.whatsapp.net"));

    whatsappd_sanitize_jid(buf, "6285726767672@c.us");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    whatsappd_sanitize_jid(buf, "6285726767672@@x");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    return 0;
}
int test_main()
{

    return test_functions();
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
