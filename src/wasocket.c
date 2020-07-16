#include <unistd.h>
#include <malloc.h>
#include <time.h>
#include <helper.h>
#include <color.h>

#include "wasocket.h"

static char *wasocket_data, *wasocket_out;
static size_t wasocket_len = 0,
              wasocket_alloc = 0,
              wasocket_out_len = 0,
              wasocket_out_alloc = 0;

uint32_t wasocket_mask()
{
    uint32_t mask = 0;
    srand(time(0) + rand());
    for (int i = 0; i < 4 / sizeof(int); i++)
    {
        ((int *)&mask)[i] = rand();
    }
    return mask;
}
static void wasocket_buf_write_int(uint8_t *data, uint8_t len)
{
    for (int i = 0; i < len; i++)
    {
        //reversed
        wasocket_out[wasocket_out_len++] = data[len - (1 + i)];
    }
}

static int wasocket_send()
{
    accent(">> %lu bytes", wasocket_out_len);
    hexdump(wasocket_out, wasocket_out_len);
    return ssl_write(wasocket_out, wasocket_out_len);
}

/* All control frames MUST have a payload length of 125 bytes or less
 * and MUST NOT be fragmented. */
static int wasocket_handle_control_frame()
{
    return 0;
}

static int wasocket_frame(enum WS_OPCODE op_code, uint64_t payload_len, uint32_t mask)
{

    // reset size
    wasocket_out_len = 0;
    // https://tools.ietf.org/html/rfc6455#page-31 fin = 1
    wasocket_out[wasocket_out_len++] = 0b1000 << 4 | (op_code & 0b1111);

    if (payload_len <= 0x7d)
    {
        // 7 bit len
        wasocket_out[wasocket_out_len++] = payload_len | (1 << 7);
    }
    else if (payload_len <= 0xffff)
    {
        payload_len &= 0xffff;
        wasocket_out[wasocket_out_len++] = 0x7E | (1 << 7);
        wasocket_buf_write_int((uint8_t *)&payload_len, 2);
        // hexdump(((char *)&payload_len), 2);
        // hexdump(wasocket_out, wasocket_out_len);
    }
    else
    {
        payload_len &= 0xffffffffffffffffLL;
        wasocket_out[wasocket_out_len++] = 0x7F | (1 << 7);
        wasocket_buf_write_int((uint8_t *)&payload_len, 8);
        // hexdump(((char *)&payload_len), 8);
        // hexdump(wasocket_out, wasocket_out_len);
    }

    // Masking
    wasocket_buf_write_int((uint8_t *)&mask, 4);
    return 0;
}

static void wasocket_send_shutdown()
{
    info("wasocket_send_shutdown");
    uint32_t mask = wasocket_mask();
    wasocket_frame(WS_OPCODE_CONNECTION, 0, mask);
    if (wasocket_send() != 6)
    {
        warn("wasocket: Send shutdown fail");
    }
}

static int wasocket_handshake(const char *host)
{
    char nonce[16], ws_key[256];

    crypto_random(nonce, 16);
    crypto_base64_encode(ws_key, 256, nonce, 16);

    wasocket_len = sprintf(wasocket_data, "GET /ws HTTP/1.1\r\n"
                                          "Host: %s\r\n"
                                          "Origin: https://%s\r\n"
                                          "Upgrade: websocket\r\n"
                                          "Connection: Upgrade\r\n"
                                          "User-Agent: ndunks-whatsappd/1.0\r\n"
                                          "Sec-WebSocket-Key: %s\r\n"
                                          "Sec-WebSocket-Version: 13\r\n\r\n",
                           host, host, ws_key);

    ssl_write(wasocket_data, wasocket_len);

    wasocket_len = ssl_read(wasocket_data, wasocket_alloc);

    return 0;
}

int wasocket_connect(const char *host)
{
    wasocket_alloc = 1024 * 4;
    wasocket_out_alloc = 1024 * 4;
    wasocket_data = malloc(wasocket_alloc);
    wasocket_out = malloc(wasocket_out_alloc);

    if (wasocket_data == NULL || wasocket_out == NULL)
    {
        err("wasocket: Fail alloc data buffer");
        CATCH_RET = 1;
        goto CATCH;
    }

    if (host == NULL)
        host = "web.whatsapp.com";

    TRY(ssl_connect(host, "443"));
    TRY(wasocket_handshake(host));

    return 0;

CATCH:
    free(wasocket_data);
    free(wasocket_out);
    return CATCH_RET;
}

int wasocket_disconnect()
{
    wasocket_send_shutdown();
    // ssl_disconnect();
    // free(wasocket_data);
    // free(wasocket_out);
    return 0;
}

char *wasocket_read()
{
    return NULL;
}