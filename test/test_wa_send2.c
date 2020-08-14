#include "proto.h"
#include "whatsappd.h"

#include "test.h"

int test_main()
{
    char rand_str[11], rand_bytes[5], text[128];
    crypto_random(rand_bytes, 5);
    helper_buf_to_hex((uint8_t *)rand_str, (uint8_t *)rand_bytes, 5);
    sprintf(text, "%s Hello from whatsapp daemon", rand_str);

    MessageKey key;
    Message msg;
    WebMessageInfo info;
    size_t buf_len, node_size, text_len = strlen(text), sent_size;
    char *bin_buf, *node_buf, tag[80] = {0},
                              number[] = "6285726501017";
    BINARY_NODE *node, child;
    buf_len = text_len + 512;

    memset(&child, 0, sizeof(BINARY_NODE));

    bin_buf = malloc(buf_len);
    node_buf = malloc(buf_len);

    key.fromMe = 1;
    wa_create_msg_id(key.id);
    wa_sanitize_jid_long(key.remoteJid, number);

    msg.conversation = (char *)text;

    info.key = &key;
    info.message = &msg;
    info.status = WEB_MESSAGE_INFO_STATUS_PENDING;
    info.messageTimestamp = time(NULL) - 1000;

    buf_set(bin_buf, buf_len);
    proto_write_WebMessageInfo(&info);

    child.tag = "message";
    child.child_type = BINARY_NODE_CHILD_BINARY;
    child.child_len = buf_idx;
    child.child.data = bin_buf;

    node = binary_node_action("relay", &child);
    node_size = binary_write(node, node_buf, buf_len);

    sprintf(tag, "%s,", key.id);
    sent_size = wasocket_send_binary(node_buf, node_size, tag, BINARY_METRIC_MESSAGE, 0);

    free(bin_buf);
    free(node_buf);
    binary_node_action_free(node);

    CHECK(sent_size == 0);
    ZERO(wa_reply_json_ok(tag));
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
    int unread_len = 0;

    while (chat != NULL)
    {
        if (chat->msg_count || chats->unread_count)
        {
            if (chat->msg_count > chat->unread_count)
                unread_len = chat->unread_count;
            else
                unread_len = chat->msg_count;
        }
        else
            unread_len = 0;
        if (unread_len)
        {
            info("UNREAD MESSAGE: %s %s %d\n%s", chat->jid, chat->name, unread_len, chat->msg[0]);
        }

        chat = chat->next;
    }
    accent("SETUP DONE -----------------------");
    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    whatsappd_free();
    return 0;
}
