#include "json.h"

JSON json[JSON_MAX_FIELDS] = {0};
int json_len = 0;

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

int64_t json_get_number(char *key)
{
    char *val;
    if ((val = json_get(key)) != NULL)
        return atoll(val);

    return 0;
}

bool json_get_bool(char *key)
{
    char *val;
    if ((val = json_get(key)) != NULL)
        return *val == 't'; //true
    return 0;
}