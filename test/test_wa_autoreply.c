#include "whatsappd.h"
#include "test.h"

int test_main()
{
    ZERO(whatsappd_autoreply_unread());
    return 0;
}

int test_setup()
{

#ifdef HEADLESS
    err("This test require working .cfg");
    return TEST_SKIP;
#endif
    ZERO(whatsappd_init(NULL));
    return 0;
}

int test_cleanup()
{
    accent("CLEANING UP..");
    whatsappd_free();
    return 0;
}
