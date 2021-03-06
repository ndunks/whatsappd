#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "util.h"

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

typedef enum BINARY_ACTION_ADD
{
    BINARY_ACTION_ADD_NONE,
    BINARY_ACTION_ADD_RELAY,
    BINARY_ACTION_ADD_UPDATE,
    BINARY_ACTION_ADD_LAST,
    BINARY_ACTION_ADD_BEFORE,
    BINARY_ACTION_ADD_AFTER,
    BINARY_ACTION_ADD_UNREAD
} BINARY_ACTION_ADD;

typedef enum BINARY_METRIC
{
    BINARY_METRIC_NONE,
    BINARY_METRIC_DEBUG_LOG,
    BINARY_METRIC_QUERY_RESUME,
    BINARY_METRIC_QUERY_RECEIPT,
    BINARY_METRIC_QUERY_MEDIA,
    BINARY_METRIC_QUERY_CHAT,
    BINARY_METRIC_QUERY_CONTACTS,
    BINARY_METRIC_QUERY_MESSAGES,
    BINARY_METRIC_PRESENCE,
    BINARY_METRIC_PRESENCE_SUBSCRIBE,
    BINARY_METRIC_GROUP,
    BINARY_METRIC_READ,
    BINARY_METRIC_CHAT,
    BINARY_METRIC_RECEIVED,
    BINARY_METRIC_PIC,
    BINARY_METRIC_STATUS,
    BINARY_METRIC_MESSAGE,
    BINARY_METRIC_QUERY_ACTIONS,
    BINARY_METRIC_BLOCK,
    BINARY_METRIC_QUERY_GROUP,
    BINARY_METRIC_QUERY_PREVIEW,
    BINARY_METRIC_QUERY_EMOJI,
    BINARY_METRIC_QUERY_MESSAGE_INFO,
    BINARY_METRIC_SPAM,
    BINARY_METRIC_QUERY_SEARCH,
    BINARY_METRIC_QUERY_IDENTITY,
    BINARY_METRIC_QUERY_URL,
    BINARY_METRIC_PROFILE,
    BINARY_METRIC_CONTACT,
    BINARY_METRIC_QUERY_VCARD,
    BINARY_METRIC_QUERY_STATUS,
    BINARY_METRIC_QUERY_STATUS_UPDATE,
    BINARY_METRIC_PRIVACY_STATUS,
    BINARY_METRIC_QUERY_LIVE_LOCATIONS,
} BINARY_METRIC;

extern const char wa_host_short[5], wa_host_long[15];
extern const char *const DICTIONARY_SINGLEBYTE[];
extern int DICTIONARY_SINGLEBYTE_LEN;
BINARY_ACTION_ADD binary_get_action_add(const char *add);
void *binary_alloc(size_t size);
void binary_free();
void binary_alloc_stat();
void binary_print_attr(BINARY_NODE *node);
char *binary_attr(BINARY_NODE *node, const char *key);
BINARY_NODE *binary_child(BINARY_NODE *node, int index);

BINARY_NODE *read_node();
BINARY_NODE *binary_read(char *src, size_t src_len);
size_t write_node(BINARY_NODE *node);
size_t binary_write(BINARY_NODE *node, char *dst, size_t dst_len);

#ifdef TEST
uint8_t read_byte();
uint16_t read_int16();
uint32_t read_int20();
uint32_t read_int32();
void binary_read_set(char *src, size_t src_len);
#endif

BINARY_NODE *binary_node_action(char *type, BINARY_NODE *child);
void binary_node_action_free(BINARY_NODE *node);