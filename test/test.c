#include "test.h"

int main(int argc, char const *argv[])
{
    info("Running test: %s", TEST);

    if (test_setup())
    {
        err("Error in test_setup");
        return 1;
    }

    test_main();

    if (ret_val == 0)
        ok("** OK **");
    else
        err("** Failed **");

    if (test_cleanup())
    {
        err("Error in test_cleanup");
        return 1;
    }

    return ret_val;
}
