#pragma once
#include <crypto.h>
#include <ssl.h>

enum WS_OPCODE {
     /** Continuation Frame */
     CONTINUATION = 0,
     /** Text Frame */
     TEXT = 1,
     /** Binary Frame */
     BINARY = 2,
     /** Connection Close Frame */
     CONNECTION = 8,
     /** Ping Frame */
     PING = 9,
     /** Pong Frame */
     PONG = 10,
};

int wasocket_connect();
int wasocket_disconnect();
