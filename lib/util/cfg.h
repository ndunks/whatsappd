#pragma once

#define CFG_SERVER_SECRET_LEN 144u
#define CFG_CLIENT_ID_LEN 16u
#define CFG_KEY_LEN 32u

typedef struct __attribute__((packed))
{
    char cfg_file_version;
    /** 16 Byte ID Auto Generated */
    char client_id[CFG_CLIENT_ID_LEN];
    /** Our key, auto generated */
    struct __attribute__((packed))
    {
        char secret[CFG_KEY_LEN], private[CFG_KEY_LEN], public[CFG_KEY_LEN];
    } keys;
    /** Tokens from server, to relogin session */
    struct __attribute__((packed))
    {
        /** Base64 **/
        char client[60], server[120], browser[180];
    } tokens;
    /** Secret from server contains encrypted aesKey and macKey generated on crypto_parse_server_keys */
    char serverSecret[CFG_SERVER_SECRET_LEN];
    /** Encrypt decrypt AES */
    char aesKey[CFG_KEY_LEN];
    /** HMAC based key to sign/verify */
    char macKey[CFG_KEY_LEN];
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
const char * cfg_file_get();

int cfg_load(CFG *cfg);
int cfg_save(CFG *cfg);