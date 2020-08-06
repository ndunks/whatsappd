#include "session.h"
#include "handler.h"

#include "test.h"

#define REQUIRE_VALID_CFG

int test_main()
{
    CHAT *chat;

    ZERO(handler_preempt());
    info("Unread count: %d", handler_unread_count);

    chat = handler_unread_chats;
    TRUTHY(chat != NULL);
    do
    {
        info("UNREAD MESSAGE: %s %s %d\n%s", chat->jid, chat->name, chat->msg_count, chat->msg[0]);
        chat = chat->next;
    } while (chat != NULL);
    accent("-----------");
    // accent("MAIN RUNTIME START");
    // ZERO(wasocket_start());
    // sleep(10);
    // wasocket_stop();
    // accent("MAIN RUNTIME Stop");
    return 0;
}

int test_setup()
{
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

    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    session_free();
    crypto_free();
    return 0;
}
