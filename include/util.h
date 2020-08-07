#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define BUF_ERR -1
#define BUF_ERR_NOT_ENOUGH -2
#define CFG_SERVER_SECRET_LEN 144u
#define CFG_CLIENT_ID_LEN 16u
#define CFG_KEY_LEN 32u

#define hexdump(buf, size)                            \
    do                                                \
    {                                                 \
        for (int i = 0; i < size; i++)                \
        {                                             \
            printf("%02x ", (unsigned char)(buf)[i]); \
        }                                             \
        printf("\n");                                 \
    } while (0)

#define TRY(expression)                         \
    if ((CATCH_RET = (expression)) != 0)        \
    {                                           \
        err(#expression ": RET %d", CATCH_RET); \
        goto CATCH;                             \
    }

#define CHECK(expression)                \
    if ((CATCH_RET = (expression)) != 0) \
        return CATCH_RET;

#define die(msg)  \
    do            \
    {             \
        err(msg); \
        exit(1);  \
    } while (0)

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
#define accent(fmt, ...) debug(COL_MAG fmt COL_NORM, ##__VA_ARGS__)

extern int CATCH_RET;
extern uint8_t *buf;
extern size_t buf_idx, buf_len;

void buf_set(char *src, size_t src_len);
uint8_t buf_read_byte();
// Big-endian read
uint16_t buf_read_int16();
// Big-endian read
uint32_t buf_read_int20();
// Big-endian read
uint32_t buf_read_int32();

uint32_t buf_read_var_int32(unsigned len, const uint8_t *data);
void buf_read_bytes(char *dst, size_t len);
bool buf_available();


typedef struct __attribute__((packed))
{
    char cfg_file_version;
    /** 16 Byte ID Auto Generated */
    char client_id[CFG_CLIENT_ID_LEN];
    /** Our key, auto generated */
    struct __attribute__((packed))
    {
        char /* secret[CFG_KEY_LEN],  */
            private[CFG_KEY_LEN],
            public[CFG_KEY_LEN];
    } keys;
    /** Tokens from server, to relogin session */
    struct __attribute__((packed))
    {
        /** Base64 **/
        char client[256], server[256], browser[256];
    } tokens;
    /** Secret from server contains encrypted aesKey and macKey generated on crypto_parse_server_keys */
    char serverSecret[CFG_SERVER_SECRET_LEN];
    /** Encrypt decrypt AES */
    // char aesKey[CFG_KEY_LEN];
    // /** HMAC based key to sign/verify */
    // char macKey[CFG_KEY_LEN];
} CFG;

/**
 * Set config file location and check RW access.
 * NULL for default = ~/.whatsappd.cfg.
 * 
 * return:
 *  0< Error
 *  0  success but not exist
 *  1  success and file exist
 */
int cfg_file(const char *file_path);
const char *cfg_file_get();
int cfg_load(CFG *cfg);
int cfg_save(CFG *cfg);
int cfg_has_credentials(CFG *cfg);

// dst len must be buf_len * 2 + 1
void helper_buf_to_hex(const uint8_t *dst, uint8_t *buf, int buf_len);
int helper_save_file(const char *path, const char *buf, size_t buf_len);
uint64_t helper_jid_to_num(const char *buf);
int helper_qrcode_show(const char *src);