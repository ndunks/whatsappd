#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/error.h>
#include <mbedtls/debug.h>

#include "color.h"
#include "ssl.h"

static mbedtls_net_context ws_net;
static mbedtls_ssl_config conf;
mbedtls_ssl_context ssl;

#ifdef MBEDTLS_DEBUG_C
static char *levelColor[] = {
    COL_RED,
    COL_YELL,
    COL_MAG,
    COL_BLUE,
    COL_CYN};
static void ssl_debug(void *ctx, int level, const char *file, int line,
                      const char *str)
{
    // const char *p, *basename;
    // (void)ctx;
    /* Extract basename from file */
    // for (p = basename = file; *p != '\0'; p++)
    // {
    //     if (*p == '/' || *p == '\\')
    //     {
    //         basename = p + 1;
    //     }
    // }

    //printf("%s%s:%04d: |%d| %s" COL_NORM, levelColor[level], basename, line, level, str);
    printf("%s%s" COL_NORM, levelColor[level], str);
}
#endif

static void ssl_init()
{
    mbedtls_net_init(&ws_net);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);

#ifdef MBEDTLS_DEBUG_C
    mbedtls_ssl_conf_dbg(&conf, ssl_debug, NULL);
    mbedtls_debug_set_threshold(4);
#endif
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

void ssl_disconnect()
{
    mbedtls_ssl_close_notify(&ssl);
    ssl_free();
}

int ssl_connect(const char *host, const char *port)
{
    struct addrinfo hints, *hostinfo = NULL, *ptr = NULL;
    char *pers = "wasocket", err_buff[255];
    int ret;

    ssl_init();

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

    mbedtls_ssl_set_bio(&ssl, &ws_net, mbedtls_net_send, mbedtls_net_recv, NULL);

    //Cipher: TLSv1.1/TLS-RSA-WITH-AES-128-CBC-SHA
    if ((ret = mbedtls_ssl_handshake(&ssl)) == 0)
    {
        accent("Cipher: %s/%s",
               mbedtls_ssl_get_version(&ssl),
               mbedtls_ssl_get_ciphersuite(&ssl));
        return 0;
    }
    mbedtls_strerror(ret, err_buff, 255);
    err("SSL Handshake fail 0x%04x %s", (unsigned int)-ret, err_buff);

exit:
    ssl_free();
    return ret;
}