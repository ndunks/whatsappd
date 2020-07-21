#include <string.h>
#include <stdio.h>
#include <wchar.h>

#include "qrcode.h"
#include "helper.h"
#include "qrcodegen.h"

static void printQr(const uint8_t qrcode[])
{
    int size = qrcodegen_getSize(qrcode);
    int border = 4;
    for (int y = -border; y < size + border; y++)
    {
        for (int x = -border; x < size + border; x++)
        {
            fputs((qrcodegen_getModule(qrcode, x, y) ? L"\u2588" : L" "), stdout);
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