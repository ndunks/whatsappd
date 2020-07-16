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
int wss_disconnect();

size_t wss_send_text(char *msg, size_t len);