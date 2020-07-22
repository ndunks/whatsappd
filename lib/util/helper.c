#include <locale.h>

#include "helper.h"
#include "qrcodegen.h"

#define HELPER_MAX_LOOP 100

int CATCH_RET = 0;

int helper_qrcode_show(const char *src)
{
    int size, border = 2;
    bool top, bot;
    const char *ptr,
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

char *helper_json_field(char **src)
{
    char *start = NULL, *ptr;
    do
    {
        if (**src == '"')
        {
            if (start)
            {
                //escaped double quotes
                if (*((*src) - 1) == '\\')
                {
                    ptr = (*src)++;
                    // shift it
                    while ((*(ptr - 1) = *ptr))
                        ptr++;
                }
                else
                { // end of field
                    **src = 0;
                    (*src)++;
                    return start;
                    break;
                }
            }
            else // start of field
                start = ++(*src);
        }
    } while (*(*src)++);

    return NULL;
}

static char helper_json_close_token(char c)
{
    switch (c)
    {
    case '{':
        return '}';
        break;
    case '[':
        return ']';
        break;
    case '"':
        return '"';
        break;
    default: // number or boolean
        return 0;
        break;
    }
}

static void helper_json_value_end(char **src)
{
    do
    {
        switch (**src)
        {
        case ' ':
        case '\r':
        case '\n':
        case '\t':
        case ',':
        case '}':
        case ']':
            *((*src)++) = 0;
        case 0:
            return;
        }
        // end of value
    } while (*(*src)++);
}
static bool helper_json_value_end_token(char **src, char close_token)
{
    char stack[64] = {0}, *token;
    int depth = 0;

    do
    {
        switch (**src)
        {
        case '\\':
            (*src)++;
            break;
        case '"':
            if (depth == 0)
            {
                if (close_token == '"')
                {
                    goto FOUND;
                }

                stack[++depth] = '"';
            }
            else if (stack[depth] == '"')
            {
                stack[depth--] = '*';
            }
            break;
        case '{':
            stack[++depth] = '}';
            break;
        case '[':
            stack[++depth] = ']';
            break;
        case '}':
        case ']':
            if (depth)
            {
                if (stack[depth] == **src)
                    stack[depth--] = '*';
                else
                {
                    err("JSON: unexpected close token: %c", **src);
                    return false;
                }
            }
            else if (close_token == **src)
            {
                goto FOUND;
            }
            else
            {
                warn("JSON: unexpected close token: %c", **src);
            }
            break;
        }
        // end of value
    } while (*(*src)++);

    return false;
FOUND:
    *(++(*src)) = 0;
    (*src)++;
    return true;
}

/* Not support nested values. Just read as-is */
char *helper_json_value(char **src)
{
    char *start = NULL, *ptr, close_token;

    do
    {
        if (**src == ':')
        {
            // Left trim
            do
                (*src)++;
            while (**src == ' ' ||
                   **src == '\t' ||
                   **src == '\r' ||
                   **src == '\n');

            // start of value
            start = *src;
            break;
        }
    } while (*(*src)++);

    close_token = helper_json_close_token(**src);

    (*src)++;

    if (close_token)
    {
        if (helper_json_value_end_token(src, close_token))
            return start;
    }
    else
    {
        helper_json_value_end(src);
        return start;
    }

    return NULL;
}

/* Simple double quote escape. not support multibytes */
char *helper_json_unescape(char *in_str)
{
    uint8_t *start, *ptr, *in = (uint8_t *)in_str;
    if (*(start = (uint8_t *)in++) == '"')
        start++;
    else
        return in_str;

    do
    {
        switch (*in)
        {
        case '\\':
            ptr = ++in; //whatever the value, skip it
            if (*ptr == 'u' || *ptr == 'U')
                continue; // unsupported unicode, keep it
            // shift it
            while ((*(ptr - 1) = *ptr))
                ptr++;

            break;

        case '\"':
            *in = 0u;

            return (char *)start;
        }
    } while (*in++);

    return (char *)start;
}

int helper_parse_init_reply(struct HELPER_JSON_INIT_REPLY *dst, const char *src)
{
    char *field, *value, *next, **dst_field = NULL;
    size_t loop = 0, len = 0;

    if (src[0] != '{')
    {
        err("Not JSON Object");
        return 1;
    }
    next = (char *)src;
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

int helper_parse_conn(struct HELPER_JSON_INIT_CONN *dst, const char *src)
{
    char *field, *value;
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
    }]
     */
    if (strncmp(src, "[\"Conn\"", 7) != 0)
    {
        err("Invalid Conn data:\n%s", src);
        return 1;
    }
    src = strchr(src, '{');
    while ((field = helper_json_field(src)))
    {
    }
    printf("SB: %c\n", *src);
    return 1;
}
