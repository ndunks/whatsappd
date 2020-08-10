#pragma once

#include <stdint.h>

#include "wss.h"
#include "binary.h"

void wasocket_setup();
char *wasocket_short_tag();
char *wasocket_tag();

size_t wasocket_send(char *data, uint len, char *tag, enum WS_OPCODE opcode);
size_t wasocket_send_text(char *data, uint len, char *tag);
size_t wasocket_send_binary(char *data, uint len, char *tag, BINARY_METRIC metric, uint8_t flag);

int wasocket_read(char **data, char **tag, ssize_t *data_size);
int wasocket_read_all(uint32_t timeout_ms);

int wasocket_start();
int wasocket_stop();
int wasocket_lock();
int wasocket_unlock();