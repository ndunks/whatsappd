#pragma once

#include <string.h>
#include <helper.h>
#include "wasocket.h"
#include "session.h"

int whatsappd_init(const char const *config_path);
void whatsappd_free();