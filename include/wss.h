#pragma once

#include "crypto.h"
#include "util.h"

int ssl_connect(const char *host, const char *port);
void ssl_disconnect();

int ssl_random(char *buf, size_t len);
int ssl_write(const char *buf, size_t size);
int ssl_read(char *buf, size_t size);
int ssl_check_read(uint32_t timeout_ms);
void ssl_error(const char *msg, int errcode);

#define WSS_NEED(len, x, x_len, x_size)            \
    while ((x_len + len) > x_size)                 \
    {                                              \
        x_size += len + 1;                         \
        x = realloc(x, x_size);                    \
        if (x == NULL)                             \
            die("wss: Fail realloc " #x);          \
        accent("wss: realloc " #x " %lu", x_size); \
    }

#define WSS_NEED_TX(len) WSS_NEED(len, wss.tx, wss.tx_len, wss.tx_size)
#define WSS_NEED_RX(len) WSS_NEED(len, wss.rx, wss.rx_len, wss.rx_size)
#define WSS_NEED_BUF(len) WSS_NEED(len, wss.buf, wss.buf_len, wss.buf_size)

#define WSS_FRAGMENT_MAX 100

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

typedef struct WSS
{
    char *tx, *rx, *buf;
    size_t rx_len, rx_size, rx_idx,
        tx_len, tx_size, tx_idx,
        buf_len, buf_size, buf_idx;
} WSS;

struct PAYLOAD
{
    char *data;
    uint64_t size;
    uint8_t frame_size;
};

typedef struct FRAME_RX
{
    uint8_t // fin,   //  1 bit
        // rsv1,      //  1 bit
        // rsv2,      //  1 bit
        // rsv3,      //  1 bit
        opcode, //  4 bits
        masked; //  1 bit
    //uint32_t mask; // 32 bits
    uint8_t payload_count;
    /* Payload data len only */
    size_t payload_size;
    struct PAYLOAD payloads[WSS_FRAGMENT_MAX];
} FRAME_RX;

WSS wss;
FRAME_RX wss_frame_rx;

uint32_t wss_mask();
size_t wss_frame(enum WS_OPCODE op_code, uint64_t payload_len, uint32_t mask);

void wss_write_chunk(uint8_t *src, size_t src_start, size_t src_end, const uint32_t *const mask);
void wss_write(uint8_t *data, size_t len, const uint32_t *const mask);
int wss_send();

int wss_connect(const char *host, const char *port, const char *path);
void wss_disconnect();
char *wss_read(size_t *data_len);

size_t wss_send_buffer(char *msg, size_t len, enum WS_OPCODE opcode);
size_t wss_send_binary(char *msg, size_t len);
size_t wss_send_text(char *msg, size_t len);