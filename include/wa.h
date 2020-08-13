#pragma once
#include <time.h>

#include "json.h"
#include "handler.h"

void wa_create_msg_id(const char *buf);
void wa_sanitize_jid(char *dst, const char *number, const char *host);
void wa_sanitize_jid_long(char *dst, const char *number);
void wa_sanitize_jid_short(char *dst, const char *number);

int wa_reply_json_ok(char *req_tag);
int wa_presence(int available);
int wa_presence_check(const char *number);
int wa_send_text(const char *number, const char *const text);
