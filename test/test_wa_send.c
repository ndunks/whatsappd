#include "proto.h"
#include "whatsappd.h"

#define TEST_REQUIRE_VALID_CFG
#include "test.h"
int test_functions()
{

    char buf[21];
    whatsappd_create_msg_id(buf);
    info("ID: %s", buf);
    TRUTHY(strlen(buf) == 20);

    whatsappd_sanitize_jid(buf, "6285726767672");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    whatsappd_sanitize_jid(buf, "62999999999@xxxx");
    ZERO(strcmp(buf, "62999999999@s.whatsapp.net"));

    whatsappd_sanitize_jid(buf, "6285726767672@c.us");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    whatsappd_sanitize_jid(buf, "6285726767672@@x");
    ZERO(strcmp(buf, "6285726767672@s.whatsapp.net"));

    return 0;
}
int test_main()
{
    char rand_str[11], rand_bytes[5], msg[128];
    crypto_random(rand_bytes, 5);
    helper_buf_to_hex((uint8_t *)rand_str, (uint8_t *)rand_bytes, 5);
    sprintf(msg, "%s Hello from whatsapp daemon", rand_str);
    CHECK(test_functions());
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
    chat = handler_unread_chats;

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
    //whatsappd_free();
    return 0;
}
