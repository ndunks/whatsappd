#include "test.h"
int ret_val;
int main(int argc, char const *argv[])
{
    char test_name[] = TEST;

    info("** TEST: %s **", test_name);

    if (test_setup())
    {
        err("Error in test_setup");
        return 1;
    }

    if ((ret_val = test_main()) != 0){
        err("** TEST: %s FAIL %d **", test_name, ret_val);
        err("-------------------------\n");
    }
    else{

        ok("** TEST: %s OK **", test_name);
        ok("-------------------------\n");
    }

    if (test_cleanup())
    {
        err("Error in test_cleanup");
        return 1;
    }

    return ret_val;
}
