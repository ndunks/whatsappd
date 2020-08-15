#pragma once

#include "wa.h"
#include "session.h"
#include "lang.h"

int whatsappd_autoreply_unread();
int whatsappd_send_info(CHAT *chat);
int whatsappd_autoreply();
int whatsappd_init(const char const *config_path);
void whatsappd_free();