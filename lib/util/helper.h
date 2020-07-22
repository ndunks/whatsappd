#pragma once
#include "string.h"
#include <stdlib.h>
#include "color.h"

#define die(msg)          \
    do                    \
    {                     \
        printf(msg "\n"); \
        exit(1);          \
    } while (0)

struct HELPER_JSON_INIT_REPLY
{
    char *status, *ref, *ttl, *update, *curr, *time;
};

struct HELPER_JSON_INIT_CONN
{
    char
        *ref,          // "1@k.."
        *wid,          // "6285726501017@c.us"
        *connected,    // true,
        *isResponse,   // "false"
        *serverToken,  // "1@+.."
        *browserToken, // "1@3.."
        *clientToken,  // "dZu.."
        *lc,           // "ID"
        *lg,           // "en"
        *locales,      // "en-ID,id-ID"
        *is24h,        // true,
        *secret,       // "s4kb.."
        *protoVersion, // [0,17]
        *binVersion,   // 10,
        *battery,      // 64,
        *plugged,      // false,
        *platform,     // "iphone"
        *features;     // {"KEY_PARTICIPANT":true,"FLAGS":"EAE..."}
};

int helper_qrcode_show(const char *src);
char *helper_json_unescape(char *str);
char *helper_json_field(char **src);
char *helper_json_value(char **src);
int helper_parse_init_reply(struct HELPER_JSON_INIT_REPLY *dst, const char *src);
int helper_parse_conn(struct HELPER_JSON_INIT_CONN *dst, const char *src);
