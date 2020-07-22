#include <qrcode.h>
#include "test.h"

int test_main()
{
    const char test[] = "1@K0X+UNPXH94guVvB2S+oOOszeMUtaOyr+zn8LTEevbJf3qRsEmHy5/Fvi+JECJS9zR0xiLHCp0wRvA==,"
                        "77jrZhfCD51b1PLGsw0MbBWQDYVFZI0pmJz9pvq81ng=,"
                        "hOxZa3unwWFZ5eY3RTK+Eg==";

    ZERO(qrcode_show(test));
    //printf(" %lc\n", QRCODE_BLACK_WHITE);
    printf("\nOK");
    printf("\xe2\x96\x84"); // black-white
    printf("\xe2\x96\x88"); // white-all
    printf("\xe2\x96\x80"); // white-black
    printf("\n-----------------------------------------\n");
    return 0;
}

int test_setup()
{
    //setlocale(LC_COLLATE, "en_US.UTF-8");
    return 0;
}

int test_cleanup()
{
    return 0;
}
