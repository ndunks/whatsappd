#pragma once

#include <sys/types.h>

void wasocket_setup();
char *wasocket_short_tag();
char *wasocket_tag();
int wasocket_send_text(char *data, uint len, char *tag);
int wasocket_read(char **data, char **tag, ssize_t *data_size);