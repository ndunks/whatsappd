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

int test_main()
{

    return test_simple_json_parser();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
