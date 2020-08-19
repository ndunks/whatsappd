#pragma once

#include "wa.h"
#include "session.h"
#include "lang.h"
#include "sender.h"

int whatsappd_send_info(CHAT *chat);
int whatsappd_autoreply();
int whatsappd_init(const char *config_path);
void whatsappd_free();
extern int whatsappd_flag;