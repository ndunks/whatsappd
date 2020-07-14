#include "test.h"

int main(int argc, char const *argv[])
{
    info("** Testing: %s **", TEST);

    if (test_setup())
    {
        err("Error in test_setup");
        return 1;
    }

    if (test_main() != 0)
        err("** Failed **");
    else
        ok("** OK **");

    if (test_cleanup())
    {
        err("Error in test_cleanup");
        return 1;
    }

    return ret_val;
}
