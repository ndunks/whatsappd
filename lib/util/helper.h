#pragma once
#include <stdio.h>
#include <unistd.h>

#define COL_RED "\x1b[31m"
#define COL_GREEN "\x1b[32m"
#define COL_YELL "\x1b[33m"
#define COL_BLUE "\x1b[34m"
#define COL_MAG "\x1b[35m"
#define COL_CYN "\x1b[36m"
#define COL_NORM "\x1b[0m"

#define debug(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define info(fmt, ...) debug(COL_CYN fmt COL_NORM, ##__VA_ARGS__)
#define ok(fmt, ...) debug(COL_GREEN fmt COL_NORM, ##__VA_ARGS__)
#define warn(fmt, ...) debug(COL_YELL fmt COL_NORM, ##__VA_ARGS__)
#define err(fmt, ...) debug(COL_RED fmt COL_NORM, ##__VA_ARGS__)
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