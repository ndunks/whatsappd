#pragma once
#define HANDLER_MAX_CHAT_MESSAGE 8

typedef int (*BINARY_NODE_HANDLE)(BINARY_NODE *);

typedef struct HANDLE
{
    const char *tag;
    BINARY_NODE_HANDLE function;

} HANDLE;

typedef struct CHAT
{
    // Phone number converted to number for fast searching
    uint64_t jid_num;
    char
        // phone number is 15, with longest host @s.whatsapp.net
        jid[32],
        // maximum wa name is 25
        name[32], *msg[HANDLER_MAX_CHAT_MESSAGE];
    uint msg_count;
    struct CHAT *next;
} CHAT;


size_t handler_unread_count;
CHAT *handler_unread_chats;
void handler_add_unread(const char *jid, const char *name, char *msg, size_t msg_len);
HANDLE *handler_get(const char *tag);
int handler_handle(BINARY_NODE *node);