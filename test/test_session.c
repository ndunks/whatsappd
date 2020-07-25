#include <sys/file.h>
#include <unistd.h>

#include <cfg.h>
#include <crypto.h>
#include <wss.h>
#include <session.h>
#include <wasocket.h>

#include "test.h"

static char *test_cfg_file = "tmp/test_session_whatsappd.cfg";

int test_new_session()
{
    CFG cfg;
    int ret;
    ret = cfg_file(test_cfg_file);

    TRUTHY(ret >= 0);

    if (ret == 1)
    {
        warn("Deleting %s", test_cfg_file);
        ZERO(unlink(test_cfg_file));
    }

    EQUAL(access(test_cfg_file, F_OK), -1);

    memset(&cfg, 0, sizeof(CFG));

    ZERO(session_init(&cfg));
    ZERO(cfg_save(&cfg));
    ZERO(access(test_cfg_file, F_OK));

    ZERO(wasocket_read_all(2000));
    session_free();

    info("tokens.client : %p %lu\n%s", cfg.tokens.client, strlen(cfg.tokens.client), cfg.tokens.client);
    info("tokens.server : %p %lu\n%s", cfg.tokens.server, strlen(cfg.tokens.server), cfg.tokens.server);
    info("tokens.browser: %p %lu\n%s", cfg.tokens.browser, strlen(cfg.tokens.browser), cfg.tokens.browser);

    FALSY(cfg.client_id == NULL);
    FALSY(cfg.serverSecret == NULL);
    TRUTHY(strlen(cfg.tokens.client) > 40);
    TRUTHY(strlen(cfg.tokens.server) > 40);
    TRUTHY(strlen(cfg.tokens.browser) > 40);

    TRUTHY(cfg_has_credentials(&cfg));

    // Default file location
    cfg_file(NULL);
    ZERO(cfg_save(&cfg));

    wasocket_read_all(3000);

    ok("test_new_session OK");
    return 0;
}

int test_resume_session()
{
    CFG cfg;

    if (cfg_file(NULL) != 1)
    {
        warn("Test resume session require credentials. skipped.");
        return 0;
    }

    memset(&cfg, 0, sizeof(CFG));

    ZERO(cfg_load(&cfg));
    ZERO(session_init(&cfg));
    ZERO(cfg_save(&cfg));

    ZERO(wasocket_read_all(3000));
    session_free();

    ok("test_resume_session OK");
    return 0;
}

int test_main()
{

    return test_new_session() || test_resume_session();

    //return test_new_session();

    //return test_resume_session();
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
