#pragma once

#include <sys/types.h>

#define QRCODE_WHITE_BLACK L'\u2580'
#define QRCODE_WHITE_ALL L'\u2588'
#define QRCODE_BLACK_WHITE L'\u2584'
#define QRCODE_BLACK_ALL ' '

int qrcode_show(const char *src);
