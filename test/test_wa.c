#include "proto.h"
#include "whatsappd.h"

#define TEST_REQUIRE_VALID_CFG
#include "test.h"

int test_main()
{

    return 0;
}

int test_setup()
{

#ifdef HEADLESS
    err("This test require working .cfg");
    return TEST_SKIP;
#endif

    CHAT *chat;
    ZERO(whatsappd_init(NULL));
    ok("Ready to handle messages!");
    chat = handler_unread_chats;

    TRUTHY(chat != NULL);
    do
    {
        info("UNREAD MESSAGE: %s %s %d\n%s", chat->jid, chat->name, chat->msg_count, chat->msg[0]);
        chat = chat->next;
    } while (chat != NULL);

    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    //whatsappd_free();
    return 0;
}
