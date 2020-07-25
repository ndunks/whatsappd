#include "json.h"
#include "color.h"

JSON json[JSON_MAX_FIELDS] = {0};
int json_len = 0;

static JSON *ptr = NULL;

static char json_parse_close_token(char c)
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

static void json_parse_value_end(char **src)
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

static int json_parse_value_end_token(char **src, char close_token)
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
                    goto FOUND;

                stack[++depth] = '"';
            }
            else if (stack[depth] == '"')
                stack[depth--] = '*';

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
                    return 1;
                }
            }
            else if (close_token == **src)
                goto FOUND;
            else
                warn("JSON: unexpected close token: %c", **src);
            break;
        }
        // end of value
    } while (*(*src)++);

    return 1;
FOUND:
    *(++(*src)) = 0;
    (*src)++;
    return 0;
}

/* Simple double quote escape. not support multibytes */
static char *json_unslash_str(char *in_str)
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

char *json_parse_key(char **src)
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

/* Not support nested values. Just read as-is */
char *json_parse_value(char **src)
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

    close_token = json_parse_close_token(**src);

    (*src)++;

    if (close_token)
    {
        if (json_parse_value_end_token(src, close_token) == 0)
            return start;
    }
    else
    {
        json_parse_value_end(src);
        return start;
    }

    return NULL;
}

int json_parse_object(char **src)
{
    char *key;
    json_len = 0;

    *src = strchr(*src, '{');

    if (*src == NULL)
    {
        err("json_parse_object: No object token");
        return 1;
    }

    (*src)++;

    while ((key = json_parse_key(src)))
    {
        if (json_len == JSON_MAX_FIELDS)
        {
            err("JSON: Not enough");
            return 1;
        }
        ptr = &json[json_len++];
        memset(ptr, 0, sizeof(JSON));
        ptr->key = key;
        ptr->value = json_parse_value(src);
        if (ptr->value == NULL)
        {
            err("Invalid JSON value!");
            return 1;
        }

        if (*ptr->value == '"')
            ptr->value = json_unslash_str(ptr->value);
        info("json: %s = %s", ptr->key, ptr->value);
    }
    return 0;
}

int json_find(char *key)
{
    int i;
    for (i = 0; i < json_len; i++)
        if (strcmp(key, json[i].key) == 0)
            return i;

    return -1;
}

int json_has(char *key)
{
    return json_find(key) >= 0;
}

char *json_get(char *key)
{
    int i = json_find(key);
    if (i < 0)
        return NULL;
    return json[i].value;
}