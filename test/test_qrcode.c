#include <wchar.h>
#include <locale.h>
#include <qrcode.h>
#include "test.h"

int test_main()
{
    const char test[] = "1@K0X+UNPXH94guVvB2S+oOOszeMUtaOyr+zn8LTEevbJf3qRsEmHy5/Fvi+JECJS9zR0xiLHCp0wRvA==,"
                        "77jrZhfCD51b1PLGsw0MbBWQDYVFZI0pmJz9pvq81ng=,"
                        "hOxZa3unwWFZ5eY3RTK+Eg==";
    char *ret = setlocale(LC_CTYPE, "C.UTF-8");
    printf("LOCALE: %s\n", ret);

    //ZERO(qrcode_show(test));

    // wprintf(L"%c %c %c %c\n", QRCODE_WHITE_BLACK,
    //         QRCODE_WHITE_ALL,
    //         QRCODE_BLACK_WHITE,
    //         QRCODE_BLACK_ALL);
    putwchar(QRCODE_BLACK_WHITE);
    wprintf(L">> \n\n\u2588\n <<\n");
    wprintf(L"HELLO WORLD\n\n\0");
    setlocale(LC_CTYPE, "");
    fflush(stdout);
    printf("\nOKKK\n\n");
    printf("\n-----------------------------------------\n\n");
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
