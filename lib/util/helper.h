#pragma once
#include <stdlib.h>

#define die(msg)          \
    do                    \
    {                     \
        printf(msg "\n"); \
        exit(1);          \
    } while (0)

typedef struct
{
    /** 16 Byte ID Auto Generated */
    char *client_id;
    /** Our key, auto generated */
    struct
    {
        char *secret, *private, *public;
    } keys;
    /** Tokens from server, to relogin session */
    struct
    {
        char *client, *server, *browser;
    } tokens;
    /** base64 secret from server contains encrypted aesKey and macKey */
    char *serverSecret;
    /** Encrypt decrypt AES */
    char *aesKey;
    /** HMAC based key to sign/verify */
    char *macKey;
} Config;

char *helper_random_bytes(size_t size);
int helper_config_write(Config *cfg, const char *file);
int helper_config_read(Config *cfg, const char *file);
void helper_config_init_default(Config *cfg);