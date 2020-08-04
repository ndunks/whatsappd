#pragma once
#include <stdlib.h>
#include <stdint.h>

#define BINARY_MALLOC_MAX 8000
typedef enum BINARY_NODE_CHILD_TYPE
{
    BINARY_NODE_CHILD_EMPTY,
    BINARY_NODE_CHILD_LIST,
    BINARY_NODE_CHILD_BINARY,
    BINARY_NODE_CHILD_STRING,
} BINARY_NODE_CHILD_TYPE;

typedef struct BINARY_NODE_ATTR
{
    char *key, *value;
} BINARY_NODE_ATTR;

typedef struct BINARY_NODE
{
    char *tag;
    BINARY_NODE_CHILD_TYPE child_type;
    int child_len, attr_len;
    BINARY_NODE_ATTR attrs[62];
    //void *childs;
    union data_or_list {
        char *data;
        struct BINARY_NODE **list;
    } child;
} BINARY_NODE;

typedef enum BINARY_TAG
{
    STREAM_END = 2,
    LIST_EMPTY = 0,
    DICTIONARY_0 = 236,
    DICTIONARY_1 = 237,
    DICTIONARY_2 = 238,
    DICTIONARY_3 = 239,
    JID_AD = 247,
    LIST_8 = 248,
    LIST_16 = 249,
    JID_PAIR = 250,
    HEX_8 = 251,
    BINARY_8 = 252,
    BINARY_20 = 253,
    BINARY_32 = 254,
    NIBBLE_8 = 255,
    SINGLE_BYTE_MAX = 256,
    PACKED_MAX = 254
} BINARY_TAG;

typedef enum BINARY_ACTION_ADD {
    BINARY_ACTION_ADD_NONE,
    BINARY_ACTION_ADD_RELAY,
    BINARY_ACTION_ADD_UPDATE,
    BINARY_ACTION_ADD_LAST,
    BINARY_ACTION_ADD_BEFORE,
    BINARY_ACTION_ADD_AFTER,
    BINARY_ACTION_ADD_UNREAD
} BINARY_ACTION_ADD;

extern const char wa_host_short[5], wa_host_long[15];
extern const char *const DICTIONARY_SINGLEBYTE[];
extern int DICTIONARY_SINGLEBYTE_LEN;
BINARY_ACTION_ADD binary_get_action_add(const char * add);
void *binary_alloc(size_t size);
void binary_free();
void binary_alloc_stat();
void binary_print_attr(BINARY_NODE *node);
char *binary_attr(BINARY_NODE *node, const char *key);
BINARY_NODE *binary_child(BINARY_NODE *node, int index);
