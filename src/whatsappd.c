#include "whatsappd.h"
#define WHATSAPPD_TEXT_MAX 1024

static char text_buf[WHATSAPPD_TEXT_MAX] = {0};
int whatsappd_flag = 0;

int whatsappd_send_info(CHAT *chat)
{
    const char *name = chat->name;

    if (name == NULL)
        name = LANG_DEAR[chat->lang];

    sprintf(text_buf, "%s %s, %s", LANG_HI[chat->lang], name, LANG_INFO[chat->lang]);
    return wa_send_text(chat->jid, text_buf);
}

int whatsappd_reply_unread()
{
    CHAT *chat;
    uint replied = 0;

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
            whatsappd_send_info(chat);
            replied++;
        }
        chat = chat->next;
    };

    chats_clear();

    if (replied > 0)
    {
        info("Replied %u chats", replied);
        wa_action_presence(1);
    }

    return 0;
}

void whatsappd_sender()
{
    pthread_mutex_lock(sender.mutex);
    if (sender.result == SENDER_RESULT_PENDING)
    {
        if (wa_send_text(sender.to, sender.txt) == 0)
            sender.result = SENDER_RESULT_OK;
        else
            sender.result = SENDER_RESULT_FAIL;

        pthread_cond_signal(sender.signal);
    }
    pthread_mutex_unlock(sender.mutex);
}

int whatsappd_autoreply()
{
    int ret = 0;
    ssize_t size;
    char *msg, *tag, watchdog_tag[] = "?,", watchdog[] = ",";
    BINARY_NODE *node;
    CHECK(sender_start());

    if (chats != NULL)
        whatsappd_reply_unread();

    while (ret >= 0)
    {
        if (whatsappd_flag)
            break;
        ret = wss_ssl_check_read(800);
        if (ret < 0)
            return ret;
        if (whatsappd_flag)
            break;
        if (ret == 0)
        {
            if (time(NULL) - wasocket_last_sent > 30)
                wasocket_send(watchdog, 1, watchdog_tag, WS_OPCODE_TEXT);
            whatsappd_sender();
            whatsappd_reply_unread();
            continue;
        }

        if (wasocket_read(&msg, &tag, &size) != 0)
            return -1;

        if (wss_frame_rx.opcode == WS_OPCODE_BINARY && size > 0)
        {
            node = binary_read(msg, size);
            handler_handle(node);
            binary_free();
        }
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
    sender_setup();
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
    whatsappd_flag = 1;
    session_free();
    crypto_free();
}

#ifndef TEST

int main(int argc, char const *argv[])
{
    // todo: arg parser for custom config location
    TRY(whatsappd_init(NULL));
    return whatsappd_autoreply();

CATCH:
    whatsappd_free();
    return CATCH_RET;
}

#endif