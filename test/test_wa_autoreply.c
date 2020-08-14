#include "session.h"
#include "handler.h"

#include "test.h"

int test_main()
{
    uint unread_len = 0;
    CHAT *chat;

    chat = chats;
    while (chat != NULL)
    {
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

    CFG cfg;
    memset(&cfg, 0, sizeof(CFG));

    ZERO(crypto_init());

    if (cfg_file(NULL) == 1)
        ZERO(cfg_load(&cfg));

    ZERO(session_init(&cfg));
    ZERO(cfg_save(&cfg));
    ZERO(handler_preempt());
    info("chats count: %lu", chats_count);
    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    session_free();
    crypto_free();
    return 0;
}
