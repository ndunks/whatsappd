#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/error.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/debug.h>

#include <base64.h>
#include <helper.h>

#include "wasocket.h"

static mbedtls_net_context ws_net;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_ssl_context ssl;
static mbedtls_ssl_config conf;
static int ret;
static int wasocket_ssl_init()
{
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_net_init(&ws_net);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_entropy_init(&entropy);
    return exit_ok;
}

static int wasocket_ssl_deinit()
{
    mbedtls_net_free(&ws_net);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return 0;
}

static int wasocket_init()
{
    struct addrinfo hints, *hostinfo, *ptr;
    const unsigned char *pers = "wasocket";

    wasocket_ssl_init();

    if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                              pers, strlen(pers)) != 0)
    {
        ret = ctr_drbg_seed_failed;
        goto exit;
    }

    if (mbedtls_ssl_config_defaults(&conf,
                                    MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0)
    {
        ret = ssl_config_defaults_failed;
        goto exit;
    }

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);

    if (mbedtls_ssl_setup(&ssl, &conf) != 0)
    {
        ret = ssl_setup_failed;
        goto exit;
    }

    /*
     * 2. Resolve address
     */
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("web.whatsapp.com", "443", &hints, &hostinfo) != 0)
        die("Can't resolve host!");
    for (ptr = hostinfo; ptr != NULL; ptr = ptr->ai_next)
    {
        if ((ws_net.fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1)
        {
            continue;
        }
        if (connect(ws_net.fd, ptr->ai_addr, ptr->ai_addrlen) == -1)
        {
            close(ws_net.fd);
            continue;
        }
        break;
    }
    if (ptr == NULL)
    {
        ret = hostname_failed;
        goto exit;
    }

    info("Connected to %s", inet_ntoa(((struct sockaddr_in *)ptr->ai_addr)->sin_addr));

    mbedtls_ssl_set_bio(&ssl, &ws_net, mbedtls_net_send, mbedtls_net_recv, NULL);
    freeaddrinfo(hostinfo);

    if ((ret = mbedtls_ssl_handshake(&ssl)) == 0)
    {
        return 0;
    }
    err("SSL Handshake fail 0x%04x", (unsigned int)-ret);
    ret = ssl_handshake_failed;

    // SSL INIT
    // Connected with TLS_CHACHA20_POLY1305_SHA256 encryption.
    // cert: /C=US/ST=California/L=Menlo Park/O=Facebook, Inc./CN=*.whatsapp.net
    // from: /C=US/O=DigiCert Inc/OU=www.digicert.com/CN=DigiCert SHA2 High Assurance Server CA
    //mbedtls_ssl_close_notify( &ssl );
exit:
    wasocket_ssl_deinit();
    return ret;
}

int wasocket_write(char *buf, size_t size)
{
    info(">>\n%s", buf);
    return mbedtls_ssl_write(&ssl, buf, size);
}

static int wasocket_handshake()
{
    unsigned char buff[1024], nonce[16], websocket_key[256];
    int i;
    size_t size;

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
        i = mbedtls_ssl_read(&ssl, buff + size, sizeof(buff) - 1 - size);
        size += i;
    } while (size < 4 && i > 0);
    buff[size] = 0;

    printf("recevied handshake result = \n%s\n", buff);

    return 0;
}

int wasocket_connect()
{
    if (wasocket_init() || wasocket_handshake())
    {
        return ret;
    }
    return 0;
}

int wasocket_disconnect()
{
    wasocket_ssl_deinit();
    // SSL_shutdown(ssl);
    // SSL_free(ssl);
    // SSL_CTX_free(ssl_ctx);
    return 0;
}