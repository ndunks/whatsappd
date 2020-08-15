#pragma once

#include <stdint.h>

typedef enum LANG
{
    LANG_EN,
    LANG_ID,
} LANG;

extern const char *LANG_DEAR[2], *LANG_HI[2], *LANG_INFO[2];

LANG lang_detect_by_num(uint64_t num);
LANG lang_detect_by_jid(const char *jid);
LANG lang_validate(unsigned lang);
