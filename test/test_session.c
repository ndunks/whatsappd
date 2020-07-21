#include <sys/file.h>
#include <unistd.h>

#include <cfg.h>
#include <crypto.h>
#include <wss.h>
#include <session.h>

#include "test.h"

static char *test_cfg_file = "tmp/test_session_whatsappd.cfg";

int test_new_session()
{
    CFG cfg;

    if (access(test_cfg_file, F_OK) == 0)
    {
        warn("Deleting %s", test_cfg_file);
        unlink(test_cfg_file);
    }

    EQUAL(access(test_cfg_file, F_OK), -1);

    ZERO(session_init(&cfg));

    FALSY(cfg.client_id == NULL);
    FALSY(cfg.serverSecret == NULL);
    TRUTHY(strlen(cfg.tokens.client));

    session_free();

    return 0;
}

int test_main()
{
    return test_new_session();
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
