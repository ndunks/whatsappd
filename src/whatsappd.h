#pragma once
#include <stdio.h>

#define COL_RED "\x1b[31m"
#define COL_GREEN "\x1b[32m"
#define COL_YELL "\x1b[33m"
#define COL_BLUE "\x1b[34m"
#define COL_NORM "\x1b[0m"

#define debug(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define info(fmt, ...) debug(COL_BLUE fmt COL_NORM, ##__VA_ARGS__)
#define ok(fmt, ...) debug(COL_GREEN fmt COL_NORM, ##__VA_ARGS__)
#define warn(fmt, ...) debug(COL_YELL fmt COL_NORM, ##__VA_ARGS__)
#define err(fmt, ...) debug(COL_RED fmt COL_NORM, ##__VA_ARGS__)

const char *whatsapp_ws_url = "wss://web.whatsapp.com/ws";
