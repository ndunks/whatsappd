#pragma once

#include <stdlib.h>

struct WASOCKET_DATA
{
    char *tag, *data;
    uint16_t data_len;
} wasocket_data;

void wasocket_setup();
char *wasocket_short_tag();
char *wasocket_tag();
int wasocket_send_text(char *data, uint len, char *tag);
