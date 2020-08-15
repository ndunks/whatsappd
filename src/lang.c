#include "lang.h"
#include "util.h"

const char
    *LANG_DEAR[] = {
        "Dear",
        "Sobat",
},
    *LANG_HI[] = {
        "Hi",
        "Hai",
},
    *LANG_INFO[] = {
        "This number cannot reply message, please visit https://netizen.name for more info.",
        "Nomor ini tidak bisa membalas pesan, silahkan kunjungi https://netizen.name untuk informasi lebih lanjut.",
};

LANG lang_validate(unsigned lang)
{
    if (lang < LANG_EN || lang > LANG_ID)
        return LANG_EN;
    return lang;
}

LANG lang_detect_by_num(uint64_t num)
{
    while (num > 99)
        num /= 10;
    switch (num)
    {
    case 62:
        return LANG_ID;

    default:
        return LANG_EN;
    }
}

LANG lang_detect_by_jid(const char *jid)
{
    int lang = 0;

    if (jid != NULL &&
        jid[0] >= '0' && jid[0] <= '9' &&
        jid[1] >= '0' && jid[1] <= '9')
    {
        lang = ((jid[0] - '0') * 10) + (jid[1] - '0');
    }

    return lang_detect_by_num(lang);
}
