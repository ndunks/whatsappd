#include <helper.h>
#include "test.h"

#define STR(str) #str

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

    return test_qrcode();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
