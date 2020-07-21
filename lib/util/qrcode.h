#pragma once

#include <sys/types.h>

#define QRCODE_WHITE_BLACK 0x2580UL
#define QRCODE_WHITE_ALL 0x2588UL
#define QRCODE_BLACK_WHITE 0x2584UL
#define QRCODE_BLACK_ALL ' '

int qrcode_show(const char *src);
