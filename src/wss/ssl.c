#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <mbedtls/ssl.h>
#include <mbedtls/error.h>
#include <mbedtls/debug.h>
#include <mbedtls/net_sockets.h>

#include "wss.h"

static mbedtls_net_context ws_net;
static mbedtls_ssl_config conf;
mbedtls_ssl_context ssl;

static void ssl_init()
{
    mbedtls_net_init(&ws_net);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
}

static void ssl_free()
{
    mbedtls_net_free(&ws_net);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
}

int ssl_write(const char *buf, size_t size)
{
    return mbedtls_ssl_write(&ssl, (const u_char *)buf, size);
}

int ssl_read(char *buf, size_t size)
{
    return mbedtls_ssl_read(&ssl, (u_char *)buf, size);
}

int ssl_check_read(uint32_t timeout_ms)
{
    return mbedtls_net_poll(&ws_net, MBEDTLS_NET_POLL_READ, timeout_ms);
}

void ssl_disconnect()
{
    mbedtls_ssl_close_notify(&ssl);
    ssl_free();
}

int ssl_connect(const char *host, const char *port)
{
    struct addrinfo hints, *hostinfo = NULL, *ptr = NULL;
    struct timeval timeout;
    int ret = 1;

    ssl_init();
    timeout.tv_sec = 7;
    timeout.tv_usec = 0;

    if (mbedtls_ssl_config_defaults(&conf,
                                    MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0)
    {
        err("ssl_config_defaults failed");
        goto exit;
    }

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, crypto_p_rng);
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);

    if (mbedtls_ssl_setup(&ssl, &conf) != 0)
    {
        err("ssl_setup failed");
        goto exit;
    }

    /*
     * 2. Resolve address
     */
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &hostinfo) != 0)
    {
        err("Can't resolve host!");
        goto exit;
    }

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

    freeaddrinfo(hostinfo);

    if (ptr == NULL)
    {
        err("Connect failed");
        goto exit;
    }

    if (setsockopt(ws_net.fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)))
    {
        warn("Fail set socket timeout");
    }
    if (setsockopt(ws_net.fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)))
    {
        warn("Fail set socket timeout");
    }

    mbedtls_ssl_set_bio(&ssl, &ws_net, mbedtls_net_send, mbedtls_net_recv, NULL);

    //Cipher: TLSv1.1/TLS-RSA-WITH-AES-128-CBC-SHA
    if ((ret = mbedtls_ssl_handshake(&ssl)) == 0)
    {
        accent("Cipher: %s/%s",
               mbedtls_ssl_get_version(&ssl),
               mbedtls_ssl_get_ciphersuite(&ssl));
        return 0;
    }

    ssl_error("SSL Handshake fail", ret);
exit:
    ssl_free();
    return ret;
}

void ssl_error(const char *msg, int errcode)
{
    char buf[255];
    mbedtls_strerror(errcode, buf, 255);
    err("%s (0x%x): %s", msg, (unsigned int)-errcode, buf);
}
