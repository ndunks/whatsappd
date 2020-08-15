#include "whatsappd.h"
#include "test.h"
#include <unistd.h>

int test_main()
{
    return whatsappd_autoreply();
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
