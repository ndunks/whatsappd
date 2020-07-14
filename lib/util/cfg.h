#pragma once

#ifndef CFG_DEFAULT_CONFIG_FILE
#define CFG_DEFAULT_CONFIG_FILE "/etc/whatsappd.cfg"
#endif

typedef struct __attribute__((packed))
{
    char cfg_file_version;
    /** 16 Byte ID Auto Generated */
    char client_id[16];
    /** Our key, auto generated */
    struct __attribute__((packed))
    {
        char secret[32], private[32], public[32];
    } keys;
    /** Tokens from server, to relogin session */
    struct __attribute__((packed))
    {
        /** Base64 **/
        char client[60], server[120], browser[180];
    } tokens;
    /** Secret from server contains encrypted aesKey and macKey */
    char serverSecret[144];
    /** Encrypt decrypt AES */
    char aesKey[32];
    /** HMAC based key to sign/verify */
    char macKey[32];
} CFG;

char * cfg_config_file;

int cfg_load(CFG *cfg);
int cfg_save(CFG *cfg);