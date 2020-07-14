#include <unistd.h>
#include <malloc.h>
#include <helper.h>
#include <color.h>

#include "wasocket.h"

static int wasocket_handshake()
{
    char buf[1024], nonce[16], ws_key[256];
    size_t size;
    hexdump(nonce, 16);
    crypto_random(nonce, 16);
    hexdump(nonce, 16);
    crypto_base64_encode(ws_key, 256, nonce, 16);

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

    return wasocket_handshake();
}

int wasocket_disconnect()
{
    ssl_disconnect();
    return 0;
}