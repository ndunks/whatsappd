#include <sys/file.h>
#include <unistd.h>

#include <util.h>
#include <crypto.h>
#include <binary.h>
#include <handler.h>

#include "test.h"
#include "data_auth.h"

int test_handler(char **encrypted_messages, CFG *cfg)
{
    BINARY_NODE *node;
    CHAT *chat, *chat2;
    int ret = 0, i;
    char file[256], output[BINARY_SAMPLE_MAX_SIZE], encrypted[BINARY_SAMPLE_MAX_SIZE], decrypted[BINARY_SAMPLE_MAX_SIZE];
    size_t output_size, encrypted_size, decrypted_size;
    TRUTHY(chats_count == 0);
    TRUTHY(chats == NULL);
    ZERO(crypto_parse_server_keys(cfg->serverSecret, cfg));
    for (i = 0; encrypted_messages[i] != NULL; i++)
    {
        strcpy(file, encrypted_messages[i]);

        accent(":: %s", file);
        load_sample(file, encrypted, BINARY_SAMPLE_MAX_SIZE, &encrypted_size);
        *(strrchr(file, '.')) = 0;
        load_sample(file, decrypted, BINARY_SAMPLE_MAX_SIZE, &decrypted_size);

        ret = crypto_decrypt_hmac(encrypted, encrypted_size, output, &output_size);
        ZERO(ret);
        ZERO(memcmp(decrypted, output, decrypted_size));
        TRUTHY(decrypted_size == output_size);
        node = binary_read(output, encrypted_size);
        ZERO(handler_handle(node));
        ok(":: %s OK", node->tag);
        binary_free();
    }

    handler_preempt_post();

    info("chats_count: %lu", chats_count);
    TRUTHY(chats_count >= 1);
    info("UNREAD MESSAGES");
    chat = chats;
    do
    {
        accent("%lu (msg: %d, unread: %d):\n%s", chat->jid_num, chat->msg_count, chat->unread_count, chat->name);
        for (i = 0; i < chat->msg_count; i++)
        {
            info(" %-2d: %s", i, chat->msg[i]);
        }
        chat = chat->next;
    } while (chat != NULL);
    chats_clear();
    return 0;
}

int test_main()
{
    char *encrypted_messages_android[] = {
        "android/1_b3d5dd17d947a3f6.--10a.bin.enc",
        "android/6_preempt-b3d5dd17d947a3f6.--10b.bin.enc",
        "android/7_b3d5dd17d947a3f6.--10c.bin.enc",
        "android/8_b3d5dd17d947a3f6.--10d.bin.enc",
        "android/9_preempt-b3d5dd17d947a3f6.--10e.bin.enc",
        NULL};
    char *encrypted_messages_iphone[] = {
        "iphone/07_preempt-1597297960-249.bin.enc",
        "iphone/08_preempt-1597297960-250.bin.enc",
        "iphone/09_1597297960-251.bin.enc",
        "iphone/10_1597297960-252.bin.enc",
        "iphone/12_1597297960-254.bin.enc",
        "iphone/15_1597297960-257.bin.enc",
        "iphone/13_1597297960-255.bin.enc",
        "iphone/14_1597297960-256.bin.enc",
        "iphone/16_1597297960-258.bin.enc",
        "iphone/11_1597297960-253.bin.enc",
        NULL};
    ZERO(test_handler(encrypted_messages_android, (void *)data_android_cfg));
    ok("test_handler ANDROID OK");
    ZERO(test_handler(encrypted_messages_iphone, (void *)data_iphone_cfg));
    ok("test_handler IPHONE OK");
    return 0;
}

int test_setup()
{

    ZERO(crypto_init());
    return 0;
}

int test_cleanup()
{
    warn("test_cleanup");
    crypto_free();
    return 0;
}
