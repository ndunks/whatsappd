#include <unistd.h>
#include <cfg.h>
#include <crypto.h>
#include <wss.h>
#include <session.h>
#include <wasocket.h>
#include "test.h"

#define REQUIRE_VALID_CFG

int test_main()
{
    ZERO(handler_preempt());
    // accent("MAIN RUNTIME START");
    // ZERO(wasocket_start());
    // sleep(10);
    // wasocket_stop();
    // accent("MAIN RUNTIME Stop");
    return 0;
}

int test_setup()
{
    CFG cfg;
    ZERO(crypto_init());

    if (cfg_file(NULL) != 1)
    {
        warn("Require valid credentials. skipped.");
        return 0;
    }

    memset(&cfg, 0, sizeof(CFG));
    accent("SETUP..");

    ZERO(cfg_load(&cfg));
    ZERO(session_init(&cfg));
    ZERO(cfg_save(&cfg));

    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    session_free();
    crypto_free();
    return 0;
}
