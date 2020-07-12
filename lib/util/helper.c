#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mbedtls/base64.h>
#include <mbedtls/ecdh.h>

#include "color.h"
#include "helper.h"

size_t helper_base64_encode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;

    mbedtls_base64_encode(
        (u_char *)dst,
        dst_len - 1,
        &written,
        (const u_char *)src,
        src_len);
    dst[written] = 0;

    return written;
}

size_t helper_base64_decode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;

    mbedtls_base64_decode(
        (u_char *)dst,
        dst_len - 1,
        &written,
        (const u_char *)src,
        src_len);
    dst[written] = 0;

    return written;
}

char *helper_random_bytes(size_t size)
{
    char *bytes = malloc(size);
    srand(time(0));
    while (size--)
    {
        bytes[size] = rand();
    }
    return bytes;
}

static size_t write_config_buf(char *str, char *buf)
{
    size_t size = strlen(str);

    if (size > 255)
        die("write config str to large");

    buf[0] = size;
    if (size > 0)
    {
        memcpy(&buf[1], str, size);
    }

    return size + 1;
}

static size_t read_config_buf(char **str, char *buf)
{
    size_t size = (u_int8_t)buf[0];

    if (size > 255)
        die("read config str to large");

    if (size > 0)
    {
        *str = malloc(size + 1);
        memcpy(*str, &buf[1], size);
        (*str)[size] = 0;
    }
    else
        *str = NULL;

    return size + 1;
}

/** return zero if OK **/
int helper_config_write(Config *cfg, const char *file)
{
    char buf[2048];
    size_t size = 0, w_size;
    FILE *f = fopen(file, "w");

    if (f == NULL)
    {
        err("Can't write %s", file);
        return -1;
    }

    size += write_config_buf(cfg->client_id, buf + size);

    size += write_config_buf(cfg->keys.secret, buf + size);
    size += write_config_buf(cfg->keys.private, buf + size);
    size += write_config_buf(cfg->keys.public, buf + size);

    size += write_config_buf(cfg->tokens.client, buf + size);
    size += write_config_buf(cfg->tokens.server, buf + size);
    size += write_config_buf(cfg->tokens.browser, buf + size);

    size += write_config_buf(cfg->serverSecret, buf + size);
    size += write_config_buf(cfg->aesKey, buf + size);
    size += write_config_buf(cfg->macKey, buf + size);

    if (cfg->client_id == NULL)
        w_size = fwrite(buf, 1, size, f);

    fclose(f);

    if (w_size != size)
    {
        err("Write config %s only %lu of %lu", file, w_size, size);
        return -1;
    }
    return 0;
}

int helper_config_read(Config *cfg, const char *file)
{
    char buf[2048];
    size_t size = 0, r_size;
    FILE *f = fopen(file, "r");

    if (f == NULL)
    {
        err("Can't read %s", file);
        return -1;
    }
    do
    {
        r_size = fread(buf + size, 1, 2048, f);
        warn("r_sz: %lu", r_size);
        size += r_size;
    } while (r_size > 0);

    fclose(f);

    if (size == 0)
    {
        err("read config no content");
        return -1;
    }
    size += read_config_buf(&cfg->client_id, buf + size);

    size += read_config_buf(&cfg->keys.secret, buf + size);
    size += read_config_buf(&cfg->keys.private, buf + size);
    size += read_config_buf(&cfg->keys.public, buf + size);

    size += read_config_buf(&cfg->tokens.client, buf + size);
    size += read_config_buf(&cfg->tokens.server, buf + size);
    size += read_config_buf(&cfg->tokens.browser, buf + size);

    size += read_config_buf(&cfg->serverSecret, buf + size);
    size += read_config_buf(&cfg->aesKey, buf + size);
    size += read_config_buf(&cfg->macKey, buf + size);
    return 0;
}

/* Own key pair */
void helper_config_init_default(Config *cfg)
{
    char *buf, buf_b64[128];
    buf = helper_random_bytes(32);
    helper_base64_encode(buf_b64, 128, buf, 32);
    free(buf);

    cfg->client_id = malloc(strlen(buf_b64));
    strcpy(cfg->client_id, buf_b64);
    // Keys
    cfg->keys.secret = helper_random_bytes(32);
}