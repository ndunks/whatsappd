#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <helper.h>
#include <color.h>

#include "wasocket.h"

static char *wss_data, *wss_out;
static size_t wss_len = 0,
              wss_alloc = 0,
              wss_out_len = 0,
              wss_out_alloc = 0;

uint32_t wss_mask()
{
    uint32_t mask = 0;
    srand(time(0) + rand());
    for (int i = 0; i < 4 / sizeof(int); i++)
    {
        ((int *)&mask)[i] = rand();
    }
    return mask;
}

static void wss_write(uint8_t *data, size_t len, uint8_t *mask)
{
    size_t i;
    if ((wss_out_len + len) > wss_out_alloc)
    {
        wss_out_alloc += 1024;
        wss_out = realloc(wss_out, wss_out_alloc);
        if (wss_out == NULL)
            die("wasocket: Fail realloc out buffer");

        warn("wasocket: realloc out buffer %lu", wss_out_alloc);
    }
    for (i = 0; i < len; i++)
    {
        wss_out[wss_out_len + i] = (data[i] ^ mask[i % 4]) & 0xff;
    }
    //memcpy(&wss_out[wss_out_len], data, len);
    wss_out_len += len;
}

static void wss_write_int(uint8_t *data, uint8_t len)
{
    for (int i = 0; i < len; i++)
    {
        //reversed
        wss_out[wss_out_len++] = data[len - (1 + i)];
    }
}

static int wss_send()
{
    warn(">> %lu bytes", wss_out_len);
    hexdump(wss_out, wss_out_len);
    //write(stderr, wss_out, wss_out_len);
    return ssl_write(wss_out, wss_out_len);
}

/* All control frames MUST have a payload length of 125 bytes or less
 * and MUST NOT be fragmented. */
static int wss_handle_control_frame()
{
    return 0;
}

static size_t wss_frame(enum WS_OPCODE op_code, uint64_t payload_len, uint32_t mask)
{

    // reset size
    wss_out_len = 0;
    // https://tools.ietf.org/html/rfc6455#page-31 fin = 1
    wss_out[wss_out_len++] = 0b1000 << 4 | (op_code & 0b1111);

    if (payload_len <= 0x7d)
    {
        // 7 bit len
        wss_out[wss_out_len++] = payload_len | (1 << 7);
    }
    else if (payload_len <= 0xffff)
    {
        payload_len &= 0xffff;
        wss_out[wss_out_len++] = 0x7E | (1 << 7);
        wss_write_int((uint8_t *)&payload_len, 2);
        // hexdump(((char *)&payload_len), 2);
        // hexdump(wss_out, wss_out_len);
    }
    else
    {
        payload_len &= 0xffffffffffffffffLL;
        wss_out[wss_out_len++] = 0x7F | (1 << 7);
        wss_write_int((uint8_t *)&payload_len, 8);
        // hexdump(((char *)&payload_len), 8);
        // hexdump(wss_out, wss_out_len);
    }

    // Masking
    wss_write_int((uint8_t *)&mask, 4);
    return wss_out_len;
}

static int wss_handshake(const char *host, const char *path)
{
    char nonce[16], ws_key[256];

    crypto_random(nonce, 16);
    crypto_base64_encode(ws_key, 256, nonce, 16);

    wss_len = sprintf(wss_data, "GET %s HTTP/1.1\r\n"
                                          "Host: %s\r\n"
                                          "Origin: https://%s\r\n"
                                          "Upgrade: websocket\r\n"
                                          "Connection: Upgrade\r\n"
                                          "User-Agent: ndunks-whatsappd/1.0\r\n"
                                          "Sec-WebSocket-Key: %s\r\n"
                                          "Sec-WebSocket-Version: 13\r\n\r\n",
                           path, host, host, ws_key);

    ssl_write(wss_data, wss_len);

    wss_len = ssl_read(wss_data, wss_alloc);

    return 0;
}

int wss_connect(const char *host, const char *port, const char *path)
{
    wss_alloc = 1024 * 4;
    wss_out_alloc = 1024 * 4;
    wss_data = malloc(wss_alloc);
    wss_out = malloc(wss_out_alloc);

    if (wss_data == NULL || wss_out == NULL)
    {
        err("wasocket: Fail alloc data buffer");
        CATCH_RET = 1;
        goto CATCH;
    }

    if (host == NULL)
        host = "web.whatsapp.com";

    if (path == NULL)
        path = "/ws";

    if (port == NULL)
        port = "443";

    TRY(ssl_connect(host, port));
    TRY(wss_handshake(host, path));

    return 0;

CATCH:
    free(wss_data);
    free(wss_out);
    return CATCH_RET;
}

int wss_disconnect()
{
    uint32_t mask = wss_mask();
    wss_frame(WS_OPCODE_CONNECTION, 0, mask);
    if (wss_send() != 6)
    {
        warn("wasocket: Send shutdown fail");
    }
    ssl_disconnect();
    free(wss_data);
    free(wss_out);
    return 0;
}

size_t wss_send_text(char *msg, size_t len)
{
    size_t frame_len;
    uint32_t mask = wss_mask();
    frame_len = wss_frame(WS_OPCODE_TEXT, len, mask);
    wss_write((uint8_t *)msg, len, (uint8_t *)&mask);
    return wss_send() - frame_len;
}

char *wss_read()
{
    int read = ssl_read(wss_data, wss_alloc);
    accent("<< %lu bytes", read);
    wss_data[read] = 0;
    return wss_data;
}