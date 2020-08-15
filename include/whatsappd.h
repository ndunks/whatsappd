#pragma once

#include "wa.h"
#include "session.h"
#include "lang.h"

int whatsappd_init(const char const *config_path);
void whatsappd_free();
int whatsappd_autoreply_unread();
int whatsappd_send_info(CHAT *chat);
