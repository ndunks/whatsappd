#include "wa.h"


// 20 bytes msg id + 1 for NULL. not support image id 16 bytes
void wa_create_msg_id(const char *str)
{
    char randBytes[10];
    memset((void *)str, 0, 21);
    crypto_random(randBytes, 10);
    helper_buf_to_hex((uint8_t *)str, (uint8_t *)randBytes, 10);
}

void wa_sanitize_jid(char *dst, const char *number, const char * host)
{
    char *at;
    strcpy(dst, number);
    at = strchr(dst, '@');

    if (at == NULL)
        strcat(dst, "@");
    else
    {
        if (strcmp(at + 1, host) == 0)
            return;

        *(at + 1) = 0;
    }

    strcat(dst, host);
}

void wa_sanitize_jid_long(char *dst, const char *number)
{
    wa_sanitize_jid(dst, number, wa_host_long);
}

void wa_sanitize_jid_short(char *dst, const char *number)
{
    wa_sanitize_jid(dst, number, wa_host_short);
}

