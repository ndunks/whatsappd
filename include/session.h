#pragma once

#include "crypto.h"
#include "util.h"
#include "json.h"
#include "wss.h"
#include "wasocket.h"
#include "session.h"

typedef struct Me
{
    char pushname[256], wid[32], platform[256];
} Me;

Me session_me;

int session_init(CFG *cfg_in);
void session_free();