#include "session.h"
#include "handler.h"

#include "test.h"

int test_main()
{
    CHAT *chat;

    chat = chats;
    do
    {
        info("UNREAD MESSAGE: %s %s %d %d", chat->jid, chat->name, chat->msg_count, chat->unread_count);
        for (int i = 0; i < chat->msg_count; i++)
        {
            info(" %-2d: %s", i, chat->msg[i]);
        }

        chat = chat->next;
    } while (chat != NULL);
    return 0;
}

int test_setup()
{

#ifdef HEADLESS
    err("This test require working .cfg");
    return TEST_SKIP;
#endif

    CFG cfg;
    ZERO(crypto_init());

    if (cfg_file(NULL) != 1)
    {
        warn("Require valid credentials. skipped.");
        return 0;
    }

    memset(&cfg, 0, sizeof(CFG));
    accent("SETUP..");

    ZERO(cfg_load(&cfg));
    ZERO(session_init(&cfg));
    ZERO(cfg_save(&cfg));
    ZERO(handler_preempt());
    info("Unread count: %lu", chats_count);
    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    session_free();
    crypto_free();
    return 0;
}
