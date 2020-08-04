#include <sys/file.h>
#include <unistd.h>

#include <cfg.h>
#include <crypto.h>
#include <binary_reader.h>
#include <helper.h>
#include <handler.h>

#include "test.h"
#include "data_auth.h"

static char *encrypted_messages[] = {
    "android/1_b3d5dd17d947a3f6.--10a.bin.enc",
    "android/6_preempt-b3d5dd17d947a3f6.--10b.bin.enc",
    "android/7_b3d5dd17d947a3f6.--10c.bin.enc",
    "android/8_b3d5dd17d947a3f6.--10d.bin.enc",
    "android/9_preempt-b3d5dd17d947a3f6.--10e.bin.enc",
    NULL};

int test_main()
{
    BINARY_NODE *node;
    CHAT *chat;
    int ret = 0, i;
    CFG *cfg = (void *)data_android_cfg;
    char file[256], output[2248], encrypted[2248], decrypted[2248];
    size_t output_size, encrypted_size, decrypted_size;

    ZERO(crypto_parse_server_keys(cfg->serverSecret, cfg));
    for (i = 0; encrypted_messages[i] != NULL; i++)
    {
        strcpy(file, encrypted_messages[i]);

        accent(":: %s", file);
        load_sample(file, encrypted, 2248, &encrypted_size);
        *(strrchr(file, '.')) = 0;
        load_sample(file, decrypted, 2248, &decrypted_size);

        ret = crypto_decrypt_hmac(encrypted, encrypted_size, output, &output_size);
        ZERO(ret);
        ZERO(memcmp(decrypted, output, decrypted_size));
        TRUTHY(decrypted_size == output_size);
        node = binary_read(output, encrypted_size);
        ZERO(handler_handle(node));
        ok(":: %s OK", node->tag);
    }

    info("handler_unread_count: %lu", handler_unread_count);
    TRUTHY(handler_unread_count == 1);
    chat = handler_unread_chats;
    TRUTHY(chat != NULL);
    do
    {
        info("UNREAD MESSAGE: %s %s %d\n%s", chat->jid, chat->name, chat->msg_count, chat->msg[0]);
        chat = chat->next;
    } while (chat != NULL);
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
