#pragma once
#include <crypto.h>
#include <ssl.h>

enum WS_OPCODE
{
    /** Continuation Frame */
    WS_OPCODE_CONTINUATION = 0,
    /** Text Frame */
    WS_OPCODE_TEXT = 1,
    /** Binary Frame */
    WS_OPCODE_BINARY = 2,
    /** Connection Close Frame */
    WS_OPCODE_CONNECTION = 8,
    /** Ping Frame */
    WS_OPCODE_PING = 9,
    /** Pong Frame */
    WS_OPCODE_PONG = 10,
};

uint32_t wss_mask();
int wss_connect(const char *host, const char *port, const char *path);
void wss_disconnect();
char *wss_read(size_t *data_len);

size_t wss_send_buffer(char *msg, size_t len, enum WS_OPCODE opcode);
size_t wss_send_binary(char *msg, size_t len);
size_t wss_send_text(char *msg, size_t len);