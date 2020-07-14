#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <cfg.h>
#include <helper.h>

#include "test.h"

int test_main()
{
    CFG cfg, *cfg2;
    memset(&cfg, 0, sizeof(CFG));

    info("cfg size: %lu", sizeof(CFG));
    cfg2 = calloc(sizeof(CFG), 1);

    cfg.cfg_file_version = 0x99;

    helper_random_bytes(16, cfg.client_id);
    ZERO(cfg_save(&cfg));
    ZERO(cfg_load(cfg2));

    TRUTHY(cfg.cfg_file_version == cfg2->cfg_file_version);
    ZERO(memcmp(cfg.client_id, cfg2->client_id, 16));

    memset(cfg2, 0, sizeof(CFG));

    helper_random_bytes(32, cfg.keys.private);
    helper_random_bytes(32, cfg.keys.public);
    helper_random_bytes(32, cfg.keys.secret);
    helper_random_bytes(144, cfg.serverSecret);
    helper_random_bytes(32, cfg.aesKey);
    helper_random_bytes(32, cfg.macKey);

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
