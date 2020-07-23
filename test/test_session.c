#include <sys/file.h>
#include <unistd.h>

#include <cfg.h>
#include <crypto.h>
#include <wss.h>
#include <session.h>

#include "test.h"

static char *test_cfg_file = "tmp/test_session_whatsappd.cfg";

int test_session_cfg()
{
    crypto_keys *keys, *keys2;
    CFG cfg;

    keys = crypto_gen_keys();
    FALSY(keys == NULL);

    ZERO(crypto_keys_store_cfg(keys, &cfg));
    crypto_keys_free(keys);
    keys2 = crypto_keys_init(cfg.keys.private, cfg.keys.public);
    ZERO(keys2 == NULL);

    ok("test_session_cfg OK");
    return 0;
}

int test_new_session()
{
    CFG cfg;
    int ret;
    ret = cfg_file(test_cfg_file);

    TRUTHY(ret >= 0);

    if (ret == 1)
    {
        warn("Deleting %s", test_cfg_file);
        unlink(test_cfg_file);
    }

    EQUAL(access(test_cfg_file, F_OK), -1);

    memset(&cfg, 0, sizeof(CFG));

    ZERO(session_init(&cfg));
    FALSY(cfg.client_id == NULL);
    FALSY(cfg.serverSecret == NULL);
    TRUTHY(strlen(cfg.tokens.client));
    TRUTHY(cfg_has_credentials(&cfg));

    ZERO(cfg_save(&cfg));
    // Default file location
    cfg_file(NULL);
    ZERO(cfg_save(&cfg));

    session_free();

    return 0;
}

int test_main()
{
    return test_session_cfg() || test_new_session();
}

int test_setup()
{
    ZERO(crypto_init());
    return 0;
}

int test_cleanup()
{
    crypto_free();
    return 0;
}
