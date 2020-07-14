#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <cfg.h>
#include <helper.h>

#include "test.h"

void simple_random_bytes(char *bytes, size_t size)
{
    srand(time(0) + size + rand());
    while (size--)
        bytes[size] = rand();
}

int test_main()
{
    CFG cfg, *cfg2;
    memset(&cfg, 0, sizeof(CFG));

    info("cfg size: %lu", sizeof(CFG));
    cfg2 = calloc(sizeof(CFG), 1);

    cfg.cfg_file_version = 0x99;

    simple_random_bytes(cfg.client_id, 16);
    ZERO(cfg_save(&cfg));
    ZERO(cfg_load(cfg2));

    TRUTHY(cfg.cfg_file_version == cfg2->cfg_file_version);
    ZERO(memcmp(cfg.client_id, cfg2->client_id, 16));

    memset(cfg2, 0, sizeof(CFG));

    simple_random_bytes(cfg.keys.private, 32);
    simple_random_bytes(cfg.keys.public, 32);
    simple_random_bytes(cfg.keys.secret, 32);
    simple_random_bytes(cfg.serverSecret, 144);
    simple_random_bytes(cfg.aesKey, 32);
    simple_random_bytes(cfg.macKey, 32);

    strcpy(cfg.tokens.client, "%#^$#%^$#&$^#&$*#&$#)(*");
    strcpy(cfg.tokens.server, "636tgyf7ryfhr7ryhf7rufhrg8rgrgj8rgj");
    strcpy(cfg.tokens.browser, "||ASDFGHJKDFKDFKDKFKDFKFDK636tgyf7ryfhr7ryhf7rufhrg8rgrgj8rgj%#^$#%^$#&$^#&$*#&$#)(*||");

    ZERO(cfg_save(&cfg));
    ZERO(cfg_load(cfg2));

    TRUTHY(cfg.cfg_file_version == cfg2->cfg_file_version);
    ZERO(memcmp(cfg.client_id, cfg2->client_id, 16));
    ZERO(memcmp(cfg.keys.private, cfg2->keys.private, 32));
    ZERO(memcmp(cfg.keys.public, cfg2->keys.public, 32));
    ZERO(memcmp(cfg.keys.secret, cfg2->keys.secret, 32));

    ZERO(memcmp(cfg.aesKey, cfg2->aesKey, 32));
    ZERO(memcmp(cfg.macKey, cfg2->macKey, 32));

    ZERO(memcmp(cfg.tokens.client, cfg2->tokens.client, 60));
    ZERO(memcmp(cfg.tokens.server, cfg2->tokens.server, 120));
    ZERO(memcmp(cfg.tokens.browser, cfg2->tokens.browser, 180));

    ZERO(strcmp(cfg.tokens.client, cfg2->tokens.client));
    ZERO(strcmp(cfg.tokens.server, cfg2->tokens.server));
    ZERO(strcmp(cfg.tokens.browser, cfg2->tokens.browser));

    free(cfg2);
    return 0;
}

int test_setup()
{
    char buf[255];
    cfg_config_file = "tmp/whatsappd.cfg";
    getcwd(buf, 255);
    info("PWD: %s", buf);
    return 0;
}

int test_cleanup()
{
    return 0;
}
