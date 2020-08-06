#pragma once

#include "util.h"
#include "handler.h"
#include "session.h"

int whatsappd_init(const char const *config_path);
void whatsappd_free();