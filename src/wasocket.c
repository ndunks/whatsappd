#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <helper.h>
#include <ssl.h>

#include "wasocket.h"

static int wasocket_handshake()
{
    char buf[1024], *nonce, ws_key[256];
    size_t size;

    nonce = helper_random_bytes(16);

    helper_base64_encode(ws_key, sizeof(ws_key), nonce, 16);

    free(nonce);

    size = sprintf(buf, "GET /ws HTTP/1.1\r\n"
                        "Host: web.whatsapp.com\r\n"
                        "Origin: https://web.whatsapp.com\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "User-Agent: ndunks-whatsappd/1.0\r\n"
                        "Sec-WebSocket-Key: %s\r\n"
                        "Sec-WebSocket-Version: 13\r\n\r\n",
                   ws_key);

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