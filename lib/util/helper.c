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
    char stack[64] = {0};
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
    char *start = NULL, close_token;

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

int helper_parse_init_reply(struct HELPER_JSON_INIT_REPLY *dst, char *src)
{
    char *field, *value, **dst_field = NULL;

    if (src[0] != '{')
    {
        err("Not JSON Object");
        return 1;
    }
    while ((field = helper_json_field(&src)))
    {
        value = helper_json_value(&src);
        if (value == NULL)
        {
            err("JSON: invalid value");
            return 1;
        }
        switch (field[0])
        {
        case 's':
            dst_field = &dst->status;
            break;
        case 'r':
            dst_field = &dst->ref;
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
            break;
        }
        // info("%s: %s (%p,%p)", field, value, dst, dst_field);
        if (dst_field == NULL)
        {
            warn("Unhandled field: %s (%s)", field, value);
        }
        else
        {
            if (*value == '"')
                value = helper_json_unescape(value);
            *dst_field = value;
            //ok("INFO: %p %p", dst->status, *dst_field);
            ok("* %s = %s", field, *dst_field);
        }
    };

    return 0;
}

int helper_parse_conn(struct HELPER_JSON_INIT_CONN *dst, char *src)
{
    char *field, *value, **dst_field = NULL;

    if (strncmp(src, "[\"Conn\"", 7) != 0)
    {
        err("Invalid Conn data:\n%s", src);
        return 1;
    }
    src = strchr(src, '{');
    while ((field = helper_json_field(&src)))
    {
        value = helper_json_value(&src);
        if (value == NULL)
        {
            err("JSON: invalid value");
            return 1;
        }

        switch (*field)
        {
        case 'b':
            switch (*(field + 1))
            {
            case 'a':
                dst_field = &dst->battery;
                break;
            case 'i':
                dst_field = &dst->binVersion;
                break;
            case 'r':
                dst_field = &dst->browserToken;
                break;
            }
            break;
        case 'c':
            switch (*(field + 1))
            {
            case 'l':
                dst_field = &dst->clientToken;
                break;
            case 'o':
                dst_field = &dst->connected;
                break;
            }
            break;

        case 'f':
            dst_field = &dst->features;
            break;
        case 'i':
            switch (*(field + 2))
            {
            case '2':
                dst_field = &dst->is24h;
                break;
            case 'R':
                dst_field = &dst->isResponse;
                break;
            }
            break;
        case 'l':
            switch (*(field + 1))
            {
            case 'c':
                dst_field = &dst->lc;
                break;
            case 'g':
                dst_field = &dst->lg;
                break;
            case 'o':
                dst_field = &dst->locales;
                break;
            }
            break;
        case 'p':
            switch (*(field + 2))
            {
            case 'a':
                dst_field = &dst->platform;
                break;
            case 'u':
                dst_field = &dst->plugged;
                break;
            case 'o':
                dst_field = &dst->protoVersion;
                break;
            }
            break;
        case 'r':
            dst_field = &dst->ref;
            break;
        case 's':
            switch (*(field + 2))
            {
            case 'c':
                dst_field = &dst->secret;
                break;
            case 'r':
                dst_field = &dst->serverToken;
                break;
            }
            break;

        case 'w':
            dst_field = &dst->wid;
            break;
        default:
            break;
        }
        if (dst_field == NULL)
        {
            warn("Unhandled field: %s (%s)", field, value);
        }
        else
        {
            if (*value == '"')
                value = helper_json_unescape(value);
            *dst_field = value;
            //ok("INFO: %p %p", dst->status, *dst_field);
            ok("* %s = %s", field, *dst_field);
        }
    }
    return 0;
}
