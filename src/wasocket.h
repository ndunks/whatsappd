#pragma once

#include <stdlib.h>

void wasocket_setup();
char *wasocket_short_tag();
char *wasocket_tag();
int wasocket_send(char *data, uint len);
