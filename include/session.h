#pragma once
#include "cfg.h"

typedef struct Me
{
    char pushname[256], wid[32], platform[256];
} Me;

Me session_me;

int session_init(CFG *cfg_in);
void session_free();