#pragma once
#include <time.h>

#include "json.h"
#include "handler.h"

typedef struct WA_PRESENCE_CHECK_RESULT
{
    char type[64];
    bool deny;
    time_t time;
} WA_PRESENCE_CHECK_RESULT;

void wa_create_msg_id(const char *buf);
void wa_sanitize_jid(char *dst, const char *number, const char *host);
void wa_sanitize_jid_long(char *dst, const char *number);
void wa_sanitize_jid_short(char *dst, const char *number);

int wa_reply_json_ok(char *req_tag);
char * wa_query_profile_pic_thumb(const char *number);
bool wa_query_exist(const char *number);
int wa_presence(int available);
int wa_presence_check(const char *number, WA_PRESENCE_CHECK_RESULT *result);
int wa_send_text(const char *number, const char *const text);
