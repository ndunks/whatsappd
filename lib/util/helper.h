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

int helper_parse_init_reply(struct HELPER_JSON_INIT_REPLY *dst, char *src);
int helper_json_unescape(char **str);
int helper_qrcode_show(const char *src);