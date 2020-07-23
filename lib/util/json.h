#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define JSON_MAX_FIELDS 100

typedef struct JSON
{
    char *key, *value;
} JSON;

JSON json[JSON_MAX_FIELDS];
int json_len;

char *json_parse_key(char **src);
char *json_parse_value(char **src);
int json_parse_object(char **src);
int json_find(char *key);
int json_has(char *key);
char *json_get(char *key);
#include <stdint.h>
