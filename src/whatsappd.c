#include "whatsappd.h"
#define WHATSAPPD_TEXT_MAX 1024

static char text_buf[WHATSAPPD_TEXT_MAX] = {0};

int whatsappd_send_info(CHAT *chat)
{
    const char *name = chat->name;

    if (name == NULL)
        name = LANG_DEAR[chat->lang];

    sprintf(text_buf, "%s %s, %s", LANG_HI[chat->lang], name, LANG_INFO[chat->lang]);
    return wa_send_text(chat->jid, text_buf);
}

int whatsappd_autoreply_unread()
{
    CHAT *chat;

    chat = chats;
    while (chat != NULL)
    {
        // WARN:
        // chat->unread_count may be greather than chat->msg_count
        // because old unread message may not sent on preempt.
        // If you want, you can query_message for that missing message
        if (chat->unread_count)
        {
            if (wa_action_read(chat->jid, chat->last_msg_id, chat->unread_count) != 0)
                warn("Fail mark read from %s", chat->jid);
            else
                whatsappd_send_info(chat);
        }

        chat = chat->next;
    };
    return 0;
}

/**
 * Login or resume session
 */
int whatsappd_init(const char *config_path)
{
    int cfg_status;
    CFG cfg;

    memset(&cfg, 0, sizeof(CFG));

    cfg_status = cfg_file(config_path);
    if (cfg_status < 0)
    {
        err("Cannot Read/Write %s", cfg_file_get());
        return 1;
    }

    if (cfg_status == 1)
        TRY(cfg_load(&cfg));

    TRY(crypto_init());
    TRY(session_init(&cfg));
    cfg_save(&cfg);
    TRY(handler_preempt());

#ifdef DEBUG
    TRY(wa_action_presence(1));
#else
    if (wa_action_presence(1) != 0)
        err("Send presence fail.");
#endif

    CATCH_RET = 0;

CATCH:
    return CATCH_RET;
}

void whatsappd_free()
{
    session_free();
    crypto_free();
}

#ifndef TEST

int main(int argc, char const *argv[])
{
    // todo: arg parser for custom config location
    TRY(whatsappd_init(NULL))

CATCH:
    whatsappd_free();
    return CATCH_RET;
}

#endif