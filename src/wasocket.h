#pragma once
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
enum exit_codes
{
    exit_ok = 0,
    ctr_drbg_seed_failed,
    ssl_config_defaults_failed,
    ssl_setup_failed,
    hostname_failed,
    socket_failed,
    connect_failed,
    x509_crt_parse_failed,
    ssl_handshake_failed,
    ssl_write_failed,
};
int wasocket_connect();
int wasocket_disconnect();
