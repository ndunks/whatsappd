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

#include "color.h"
#include "ssl.h"

static mbedtls_net_context ws_net;
static mbedtls_entropy_context entropy;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_ssl_config conf;
mbedtls_ssl_context ssl;

static void ssl_init()
{
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_net_init(&ws_net);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_entropy_init(&entropy);
}

static void ssl_deinit()
{
    mbedtls_net_free(&ws_net);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

int ssl_random(char *buf, size_t len)
{
    return mbedtls_ctr_drbg_random(conf.p_rng, (u_char *)buf, len);
}

int ssl_write(const char *buf, size_t size)
{
    info(">>\n%s", buf);
    return mbedtls_ssl_write(&ssl, (const u_char *)buf, size);
}

int ssl_read(char *buf, size_t size)
{
    int read;
    read = mbedtls_ssl_read(&ssl, (u_char *)buf, size - 1);
    buf[read] = 0;
    accent("<<\n%s", buf);
    return read;
}

void ssl_disconnect()
{
    mbedtls_ssl_close_notify(&ssl);
    ssl_deinit();
}

int ssl_connect(const char *host, const char *port)
{
    struct addrinfo hints, *hostinfo = NULL, *ptr = NULL;
    char *pers = "wasocket", err_buff[255];
    int ret;

    ssl_init();

    if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                              (const u_char *)pers, strlen(pers)) != 0)
    {
        err("ctr_drbg_seed failed");
        goto exit;
    }

    if (mbedtls_ssl_config_defaults(&conf,
                                    MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0)
    {
        err("ssl_config_defaults failed");
        goto exit;
    }

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
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
    mbedtls_strerror( ret, err_buff, 255 );
    err("SSL Handshake fail 0x%04x %s", (unsigned int)-ret, err_buff);

exit:
    ssl_deinit();
    return ret;
}