#include <string.h>
#include <stdio.h>
#include <locale.h>

#include "qrcode.h"
#include "helper.h"
#include "qrcodegen.h"

static void printQr(const uint8_t qrcode[])
{
    int size = qrcodegen_getSize(qrcode);
    int border = 2;
    bool top, bot;
    char *ptr,
        black_white[] = "\xe2\x96\x84", // U+0x2584
        white_all[] = "\xe2\x96\x88",   // U+0x2588
        white_black[] = "\xe2\x96\x80", // U+0x2580
        black_all[] = " ";

    setlocale(LC_CTYPE, "");

    for (int y = -border; y < size + border; y += 2)
    {
        for (int x = -border; x < size + border; x++)
        {
            top = qrcodegen_getModule(qrcode, x, y);
            bot = qrcodegen_getModule(qrcode, x, y + 1);
            if (top && bot)
                ptr = white_all;
            else if (top)
                ptr = white_black;
            else if (bot)
                ptr = black_white;
            else
                ptr = black_all;
            fputs(ptr, stdout);
        }
        fputs("\n", stdout);
    }
    fputs("\n", stdout);
}

int qrcode_show(const char *src)
{
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX],
        tempBuffer[qrcodegen_BUFFER_LEN_MAX];

    if (qrcodegen_encodeText(src, tempBuffer, qrcode,
                             qrcodegen_Ecc_LOW, 5, 16, qrcodegen_Mask_AUTO, true))
    {
        printQr(qrcode);
        return 0;
    };

    return 1;
}

void qrcode_free()
{
}