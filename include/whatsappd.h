#pragma once

#include <helper.h>
#include <crypto.h>

#include "session.h"
#include "handler.h"


int whatsappd_init(const char const *config_path);
void whatsappd_free();