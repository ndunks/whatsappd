#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <util/base64.h>
#include <util/helper.h>

#include "wasocket.h"

static int ws_fd;
static SSL_CTX *ssl_ctx;
static SSL *ssl;

static int wasocket_init()
{
    struct addrinfo hints, *hostinfo, *ptr;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("web.whatsapp.com", "443", &hints, &hostinfo) != 0)
        die("Can't resolve host!");

    for (ptr = hostinfo; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((ws_fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
        {
            continue;
        }
        if (connect(ws_fd, ptr->ai_addr, ptr->ai_addrlen) == -1)
        {
            close(ws_fd);
            continue;
        }
        break;
    }

    freeaddrinfo(hostinfo);
    if (ptr == NULL)
        return 1;
    // SSL INIT
    // Connected with TLS_CHACHA20_POLY1305_SHA256 encryption.
    // cert: /C=US/ST=California/L=Menlo Park/O=Facebook, Inc./CN=*.whatsapp.net
    // from: /C=US/O=DigiCert Inc/OU=www.digicert.com/CN=DigiCert SHA2 High Assurance Server CA

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    if (ssl_ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        return 2;
    }
    ssl = SSL_new(ssl_ctx);
    SSL_set_fd(ssl, ws_fd);
    if (SSL_connect(ssl) == -1)
    {
        ERR_print_errors_fp(stderr);
        return 3;
    }
    info("SSL: %s", SSL_get_cipher(ssl));

    return 0;
}

int wasocket_write(char *buf, size_t size)
{
    info(">>\n%s", buf);
    return SSL_write(ssl, buf, size);
}

static int wasocket_handshake()
{
    char buff[1024], nonce[16], websocket_key[256];
    int i;
    size_t size;
    srand(time(NULL));
    for (i = 0; i < sizeof(nonce); i++)
    {
        nonce[i] = (unsigned char)(rand() & 0xff);
    }
    base64_encode(nonce, sizeof(nonce), websocket_key, sizeof(websocket_key));

    size = sprintf(buff, "GET /ws HTTP/1.1\r\n"
                         "Host: web.whatsapp.com\r\n"
                         "Origin: https://web.whatsapp.com\r\n"
                         "Upgrade: websocket\r\n"
                         "Connection: Upgrade\r\n"
                         "Sec-WebSocket-Key: %s\r\n"
                         "Sec-WebSocket-Version: 13\r\n\r\n",
                   websocket_key);

    wasocket_write(buff, size);
    size = 0;
    i = 0;

    do
    {
        i = SSL_read(ssl, buff + size, sizeof(buff) - 1 - size);
        size += i;
    } while (size < 4 && i > 0);
    buff[size] = 0;

    printf("recevied handshake result = \n%s\n", buff);

    return 0;
}

int wasocket_connect()
{
    return wasocket_init() || wasocket_handshake();
}

int wasocket_disconnect()
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ssl_ctx);
    return close(ws_fd);
}