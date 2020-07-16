#include <sys/file.h>
#include <unistd.h>

#include <whatsappd.h>

#include "test.h"

int test_new_session()
{
    char *cfg_file = "tmp/test_session_whatsappd.cfg";
    if (access(cfg_file, F_OK) == 0)
    {
        warn("Deleting %s", cfg_file);
        unlink(cfg_file);
    }

    EQUAL(access(cfg_file, F_OK), -1);

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
