#include <unistd.h>
#include <stdint.h>
#include <mbedtls/base64.h>
#include <helper.h>
#include <ssl.h>

#include "wasocket.h"

static int wasocket_handshake()
{
    char buf[1024], nonce[16], websocket_key[256];
    int i;
    size_t size, w_size;

    ssl_random(nonce, 16);
    mbedtls_base64_encode((unsigned char *)websocket_key,
                          sizeof(websocket_key),
                          &w_size,
                          (const unsigned char *)nonce,
                          sizeof(nonce));

    size = sprintf(buf, "GET /ws HTTP/1.1\r\n"
                        "Host: web.whatsapp.com\r\n"
                        "Origin: https://web.whatsapp.com\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Key: %s\r\n"
                        "Sec-WebSocket-Version: 13\r\n\r\n",
                   websocket_key);

    ssl_write(buf, size);

    size = ssl_read(buf, sizeof(buf));

    return 0;
}

int wasocket_connect()
{
    if (ssl_connect("web.whatsapp.com", "443"))
        return 1;

    wasocket_handshake();

    return 0;
}

int wasocket_disconnect()
{
    ssl_disconnect();
    return 0;
}