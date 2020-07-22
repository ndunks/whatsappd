#include <helper.h>
#include "test.h"
int test_simple_json_parser()
{

    struct HELPER_JSON_INIT_REPLY parsed;
    char str[] = "{\"status\":200,"
                 "\"ref\":\"6CtS+A==\","
                 "\"ttl\":20000,"
                 "\"update\":false,"
                 "\"curr\":\"2.2029.4\","
                 "\"time\":1595261640005.0}";

    memset(&parsed, 0, sizeof(parsed));

    ZERO(helper_parse_init_reply(&parsed, str));

    ZERO(strncmp(parsed.status, "200", 4));
    ZERO(strncmp(parsed.ref, "\"6CtS+A==\"", 11));
    ZERO(strncmp(parsed.ttl, "20000", 6));
    ZERO(strncmp(parsed.update, "false", 6));
    ZERO(strncmp(parsed.curr, "\"2.2029.4\"", 11));
    ZERO(strncmp(parsed.time, "1595261640005.0", 16));
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

    return test_simple_json_parser() || test_qrcode();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
