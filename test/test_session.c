#include <whatsappd.h>

#include "test.h"

int test_new_session()
{
    char *cfg_file = "tmp/whatsappd.cfg";
    ZERO(whatsappd_init(cfg_file));

    whatsappd_free();
    return 0;
}

int test_main()
{
    return test_new_session();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
