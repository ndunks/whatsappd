#include <unistd.h>
#include <malloc.h>
#include <helper.h>
#include <color.h>
#include <sys/time.h>

#include "wasocket.h"

static char *wasocket_data;
static size_t wasocket_len = 0,
              wasocket_alloc = 0;

/* All control frames MUST have a payload length of 125 bytes or less
 * and MUST NOT be fragmented. */
static int wasocket_handle_control_frame()
{
    return 0;
}

static void wasocket_send_shutdown()
{
    info("wasocket_send_shutdown");
    uint8_t data[6];
    ssize_t i = 0, n;
    int mask_int;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec * tv.tv_usec);
    mask_int = rand();
    memcpy(data + 2, &mask_int, sizeof(mask_int));
    data[0] = 0x88;
    data[1] = 0x80;
    n = ssl_write(data, 6);
    if (n < 0)
    {
        warn("wasocket: Send shutdown fail");
    }
}

static int wasocket_handshake()
{
    char nonce[16], ws_key[256];

    crypto_random(nonce, 16);
    crypto_base64_encode(ws_key, 256, nonce, 16);

    wasocket_len = sprintf(wasocket_data, "GET /ws HTTP/1.1\r\n"
                                          "Host: web.whatsapp.com\r\n"
                                          "Origin: https://web.whatsapp.com\r\n"
                                          "Upgrade: websocket\r\n"
                                          "Connection: Upgrade\r\n"
                                          "User-Agent: ndunks-whatsappd/1.0\r\n"
                                          "Sec-WebSocket-Key: %s\r\n"
                                          "Sec-WebSocket-Version: 13\r\n\r\n",
                           ws_key);

    ssl_write(wasocket_data, wasocket_len);

    wasocket_len = ssl_read(wasocket_data, wasocket_alloc);

    return 0;
}

int wasocket_connect()
{
    wasocket_alloc = 1024 * 4;
    wasocket_data = malloc(wasocket_alloc);

    if (wasocket_data == NULL)
    {
        err("wasocket: Fail alloc data buffer");
        return 1;
    }

    if (ssl_connect("web.whatsapp.com", "443"))
        return 1;

    return wasocket_handshake();
}

int wasocket_disconnect()
{
    wasocket_send_shutdown();
    ssl_disconnect();
    free(wasocket_data);
    return 0;
}

char *wasocket_read()
{
    return NULL;
}