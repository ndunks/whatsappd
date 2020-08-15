#pragma once

#include <stdint.h>

#include "wss.h"
#include "binary.h"

#define EPHEMERAL_IGNORE (1 << 7)
#define EPHEMERAL_ACKREQUEST (1 << 6)
#define EPHEMERAL_AVAILABLE (1 << 5)
#define EPHEMERAL_NOTAVAILABLE (1 << 4)
#define EPHEMERAL_EXPIRES (1 << 3)
#define EPHEMERAL_SKIPOFFLINE (1 << 2)

extern time_t wasocket_last_sent;

void wasocket_setup();
char *wasocket_short_tag();
char *wasocket_tag();

size_t wasocket_send(char *data, uint len, char *tag, enum WS_OPCODE opcode);
size_t wasocket_send_text(char *data, uint len, char *tag);
size_t wasocket_send_binary(char *data, uint len, char *tag, BINARY_METRIC metric, uint8_t flag);

int wasocket_read(char **data, char **tag, ssize_t *data_size);
int wasocket_read_all(uint32_t timeout_ms);
char *wasocket_read_reply(char *req_tag);
size_t wasocket_read_command_reply(const char *type, char *buf, size_t buf_len);

int wasocket_start();
int wasocket_stop();
int wasocket_lock();
int wasocket_unlock();