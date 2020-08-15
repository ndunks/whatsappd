#include "chats.h"

// Unread chats count
size_t chats_count = 0;
// Get FIFO unread chats
CHAT *chats = NULL;

CHAT *chats_get(const char *jid)
{
    uint64_t jid_num;
    CHAT *chat = chats;
    jid_num = helper_jid_to_num(jid);

    while (chat != NULL)
    {
        if (chat->jid_num == jid_num)
            return chat;
        chat = chat->next;
    }

    return NULL;
}

CHAT *chats_add_unread(const char *jid, const Message *msg)
{
    uint64_t jid_num;
    CHAT *tail = NULL, *chat = chats;
    jid_num = helper_jid_to_num(jid);

    while (chat != NULL)
    {
        if (chat->jid_num == jid_num)
            break;
        tail = chat;
        chat = chat->next;
    }

    if (chat == NULL)
    {
        chat = calloc(sizeof(CHAT), 1);
        if( chat == NULL ){
            err("chats_add_unread: calloc fail!");
            return NULL;
        }
        strcpy(chat->jid, jid);
        chat->jid_num = jid_num;
        chat->lang = lang_detect_by_jid(jid);

        if (tail != NULL)
            tail->next = chat;
        else
            chats = chat;

        chats_count++;
    }

    if (msg != NULL)
        chats_add_msg(chat, msg);

    return chat;
}

void chats_free(CHAT *chat)
{
    for (int i = 0; i < chat->msg_count; i++)
    {
        if (chat->msg[i] != NULL)
            free(chat->msg[i]);
    }

    free(chat);
    chats_count--;
}

void chats_clear()
{
    CHAT *next;
    while (chats != NULL)
    {
        next = chats->next;
        chats_free(chats);
        chats = next;
    }
}

void chats_add_msg(CHAT *chat, const Message *msg)
{
    int i;
    char *msg_txt = NULL;
    size_t msg_len = 0;

    if (msg == NULL)
    {
        warn("chats_add_msg: NULL msg");
        return;
    }

    // Keep add it even is null, so the unread count keep match
    // NULL conversation mean an media item, we don't care.
    if (msg->conversation == NULL)
    {
        warn("chats_add_msg: NULL conversation");
    }
    else
    {
        msg_len = strlen(msg->conversation) + 1;
        msg_txt = malloc(msg_len);
        strcpy(msg_txt, msg->conversation);
    }

    if (chat->msg_count == HANDLER_MAX_CHAT_MESSAGE)
    {
        warn("Too many unread msg in chat, msg truncated.");
        free(chat->msg[0]);
        chat->msg_count = HANDLER_MAX_CHAT_MESSAGE - 1;
        for (i = 0; i < HANDLER_MAX_CHAT_MESSAGE - 2; i++)
        {
            chat->msg[i] = chat->msg[i + 1];
        }
    }
    chat->msg[chat->msg_count++] = msg_txt;
}
