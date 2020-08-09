#pragma once

#include "util.h"
#include "handler.h"
#include "session.h"

int whatsappd_init(const char const *config_path);
void whatsappd_free();
void whatsappd_create_msg_id(const char *buf);
void whatsappd_sanitize_jid(char *dst, const char *number);
int whatsappd_send_text(const char *number, const char *const text);