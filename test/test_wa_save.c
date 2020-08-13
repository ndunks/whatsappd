/**
 * Test to store binary message (encrypted & plain). with keys also
 * Example to use it
 * make test TEST=wa_save SAVE_MSG=test/binary-sample/iphone
 */

#include "session.h"
#include "test.h"

int test_main()
{
    CFG cfg;

    memset(&cfg, 0, sizeof(CFG));
    if (cfg_file(NULL) == 1)
        ZERO(cfg_load(&cfg));

    ZERO(session_init(&cfg));
    ZERO(cfg_save(&cfg));

    ZERO(wasocket_read_all(3000));

    session_free();

    return 0;
}

int test_setup()
{
#ifdef HEADLESS
    err("This test require user to scan qrcode");
    return TEST_SKIP;
#endif
#ifndef SAVE_MSG
    err("no SAVE_MSG defined");
    return TEST_SKIP;
#endif

    ZERO(crypto_init());
    return 0;
}

int test_cleanup()
{
    warn("test_cleanup");
    crypto_free();
    return 0;
}
