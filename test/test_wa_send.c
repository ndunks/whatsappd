#include "proto.h"
#include "whatsappd.h"

#include "test.h"

int test_main()
{
    char rand_str[11], rand_bytes[5], msg[128];
    crypto_random(rand_bytes, 5);
    helper_buf_to_hex((uint8_t *)rand_str, (uint8_t *)rand_bytes, 5);
    sprintf(msg, "%s Hello from whatsapp daemon", rand_str);

    ZERO(whatsappd_send_text("6285726501017", msg));
    return 0;
}

int test_setup()
{

#ifdef HEADLESS
    err("This test require working .cfg");
    return TEST_SKIP;
#endif
    //crypto_init();
    CHAT *chat;
    ZERO(whatsappd_init(NULL));
    ok("Ready to handle messages!");
    chat = chats;

    TRUTHY(chat != NULL);
    do
    {
        info("UNREAD MESSAGE: %s %s %d\n%s", chat->jid, chat->name, chat->msg_count, chat->msg[0]);
        chat = chat->next;
    } while (chat != NULL);
    accent("------------------------------");
    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    whatsappd_free();
    return 0;
}
