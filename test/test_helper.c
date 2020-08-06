#include <util.h>
#include "test.h"

#define STR(str) #str

int test_jid()
{
    uint64_t val;
    val = helper_jid_to_num("0@g.us");
    TRUTHY(val == 0);

    val = helper_jid_to_num("1230@g.us");
    TRUTHY(val == 1230);

    val = helper_jid_to_num("6285799999999@g.us");
    TRUTHY(val == 6285799999999llu);

    val = helper_jid_to_num("999999999999999999@s.whatsapp.net");
    TRUTHY(val == 999999999999999999llu);
    return 0;
}

int test_qrcode()
{
    const char test[] = "1@K0X+UNPXH94guVvB2S+oOOszeMUtaOyr+zn8LTEevbJf3qRsEmHy5/Fvi+JECJS9zR0xiLHCp0wRvA==,"
                        "77jrZhfCD51b1PLGsw0MbBWQDYVFZI0pmJz9pvq81ng=,"
                        "hOxZa3unwWFZ5eY3RTK+Eg==";

    ZERO(helper_qrcode_show(test));
    return 0;
}

int test_main()
{
    return test_jid() || test_qrcode();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
