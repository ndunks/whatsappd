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
    if (jid == (void *)0)
        return LANG_EN;

    if (jid[0] == '6' && jid[1] == '2')
        return LANG_ID;

    return LANG_EN;
}

LANG lang_validate(unsigned lang)
{
    if (lang < LANG_EN || lang > LANG_ID)
        return LANG_EN;
    return lang;
}
