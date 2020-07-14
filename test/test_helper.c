#include <helper.h>
#include "test.h"

static int test_base64()
{
    const char *str = "Hello World", *str_b64 = "SGVsbG8gV29ybGQ=";
    char buf[255];

    hexdump(str, 12);
    hexdump(str_b64, 17);

    helper_base64_encode(buf, 255, str, strlen(str));
    info("b64: %s", buf);
    ZERO(strcmp(buf, str_b64));

    helper_base64_decode(buf, 255, str_b64, strlen(str_b64));
    info("str: %s", buf);
    ZERO(strcmp(buf, str));
    return 0;
}

int test_main()
{

    return test_base64();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
