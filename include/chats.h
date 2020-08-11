#pragma once
#include "util.h"
#include "proto.h"

#define HANDLER_MAX_CHAT_MESSAGE 8

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
    // count value from handle_response -> handle_chat
    uint unread_count;
    struct CHAT *next;
} CHAT;

extern size_t chats_count;
extern CHAT *chats;
CHAT *chats_get(const char *jid);
CHAT *chats_add_unread(const char *jid, const char *name, const Message *msg);
void chats_add_msg(CHAT *chat, const Message *msg);
