#include "whatsappd.h"
#include "test.h"

int test_main()
{
    uint unread_len = 0;
    CHAT *chat;

    chat = chats;
    while (chat != NULL)
    {
        // WARN:
        // chat->unread_count may be greather than chat->msg_count
        // because old unread message may not sent on preempt.
        // If you want, you can query_message for that missing message
        if (chat->msg_count || chats->unread_count)
        {
            if (chat->msg_count < chat->unread_count)
                unread_len = chat->msg_count;
            else
                unread_len = chat->unread_count;
        }
        else
            unread_len = 0;

        if (unread_len)
        {
            info("UNREAD MESSAGE: %s %s %d %d", chat->jid, chat->name, chat->msg_count, chat->unread_count);
            for (int i = 0; i < unread_len; i++)
            {
                info(" %-2d: %s", i, chat->msg[i]);
            }
        }

        if (chat->unread_count)
        {
            wa_action_read(chat->jid, chat->last_msg_id, chat->unread_count);
        }

        chat = chat->next;
    };
    return 0;
}

int test_setup()
{

#ifdef HEADLESS
    err("This test require working .cfg");
    return TEST_SKIP;
#endif
    ZERO(whatsappd_init(NULL));
    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    whatsappd_free();
    return 0;
}
