#include "test.h"

int main(int argc, char const *argv[])
{
    char *test_error;
    info("Running test: %s", TEST);

    if (test_setup())
    {
        err("Error in test_setup");
        return 1;
    }

    test_error = test_main();

    if (test_error)
    {
        err("Failed:\n%s", test_error);
    }

    if (test_cleanup())
    {
        err("Error in test_cleanup");
        return 1;
    }

    return test_error != 0;
}
