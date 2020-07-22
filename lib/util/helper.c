#include "helper.h"
#include <locale.h>
#include "qrcodegen.h"

#define HELPER_MAX_LOOP 100

int CATCH_RET = 0;

int helper_json_next_field(char *ptr)
{
    char *start = ptr;
    do
    {
        switch (*ptr)
        {
        case 0:
            /* code */
            return 0;
        case '\\':

            break;
        default:
            break;
        }

    } while (*ptr++);

    return ptr - start;
}

/* Simple double quote escape. not support multibytes */
char *helper_json_unescape(char *in)
{
    char *start, *ptr;
    if (*(start = in++) == '"')
        start++;
    else
        return in;
    do
    {
        switch (*in)
        {
        case '\\':
            ptr = ++in; //whatever the value, skip it
            if (*ptr == 'u' || *ptr == 'U')
                continue; // unsupported unicode, keep it
            // shift it
            while ((*(ptr - 1) = *ptr++))
                ;
            break;

        case '\"':
            *in = 0;
            return start;
        }
    } while (*in++);

    return start;
}

int helper_parse_init_reply(struct HELPER_JSON_INIT_REPLY *dst, char *src)
{
    char *field, *value, *next, **dst_field = NULL;
    size_t loop = 0, len = 0;

    if (src[0] != '{')
    {
        err("Not JSON Object");
        return 1;
    }
    next = src;
    do
    {
        dst_field = NULL;
        field = strchr(next, '"') + 1;
        //field len
        len = strcspn(field, "\"");
        field[len] = 0;

        value = field + len + 2;
        len = strcspn(value, ",}");
        value[len] = 0;
        next = value + len + 1;

        switch (field[0])
        {
        case 's':
            dst_field = &dst->status;
            break;
        case 'r':
            dst_field = &dst->ref;
            value = helper_json_unescape(value);
            break;
        case 't':
            if (field[1] == 'i')
                dst_field = &dst->time;
            else
                dst_field = &dst->ttl;
            break;
        case 'u':

            dst_field = &dst->update;
            break;
        case 'c':
            dst_field = &dst->curr;
            value = helper_json_unescape(value);
            break;
        }
        // info("%s: %s (%p,%p)", field, value, dst, dst_field);
        if (dst_field == NULL)
        {
            warn("Unhandled field: %s (%s)", field, value);
        }
        else
        {
            *dst_field = value;
            //ok("INFO: %p %p", dst->status, *dst_field);
            ok("* %s = %s", field, *dst_field);
        }

        accent("V: %s :: %s", field, value);

        if ((++loop) > HELPER_MAX_LOOP)
        {
            err("HELPER_MAX_LOOP REACHED!");
            return 1;
        }

    } while (next[0]);

    return 0;
}

int helper_qrcode_show(const char *src)
{
    int size, border = 2;
    bool top, bot;
    char *ptr,
        black_white[] = "\xe2\x96\x84", // U+0x2584
        white_all[] = "\xe2\x96\x88",   // U+0x2588
        white_black[] = "\xe2\x96\x80", // U+0x2580
        black_all[] = " ";
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX],
        tempBuffer[qrcodegen_BUFFER_LEN_MAX];

    if (!qrcodegen_encodeText(src, tempBuffer, qrcode,
                              qrcodegen_Ecc_LOW, 5, 16,
                              qrcodegen_Mask_AUTO, true))
    {
        return 1;
    };

    size = qrcodegen_getSize(qrcode);
    setlocale(LC_CTYPE, "");

    for (int y = -border; y < size + border; y += 2)
    {
        for (int x = -border; x < size + border; x++)
        {
            top = qrcodegen_getModule(qrcode, x, y);
            bot = qrcodegen_getModule(qrcode, x, y + 1);
            if (top && bot)
                ptr = white_all;
            else if (top)
                ptr = white_black;
            else if (bot)
                ptr = black_white;
            else
                ptr = black_all;
            fputs(ptr, stdout);
        }
        fputs("\n", stdout);
    }
    fputs("\n", stdout);
    return 0;
}

int helper_parse_init_conn()
{
    /*
     ["Conn",{
         "ref":"1@k..",
         "wid":"6285726501017@c.us",
         "connected":true,
         "isResponse":"false",
         "serverToken":"1@+..",
         "browserToken":"1@3..",
         "clientToken":"dZu..",
         "lc":"ID",
         "lg":"en",
         "locales":"en-ID,id-ID",  <-- watch this have comma
         "is24h":true,
         "secret":"s4kb..",
         "protoVersion":[0,17],  <-- watch this have comma
         "binVersion":10,
         "battery":64,
         "plugged":false,
         "platform":"iphone",
         "features":{"KEY_PARTICIPANT":true,"FLAGS":"EAE..."}
     */
}