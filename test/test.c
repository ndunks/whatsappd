#include "test.h"
#include <sys/time.h>

int ret_val;
int main(int argc, char const *argv[])
{
    char test_name[] = TEST;
    struct timeval start, stop;
    long secs_used, micros_used;

    info("** TEST: %s **", test_name);

    if (test_setup())
    {
        err("Error in test_setup");
        return 1;
    }

    gettimeofday(&start, NULL);
    ret_val = test_main();
    gettimeofday(&stop, NULL);
    secs_used = (stop.tv_sec - start.tv_sec); //avoid overflow by subtracting first
    micros_used = ((secs_used * 1000000) + stop.tv_usec) - (start.tv_usec);

    if (ret_val != 0)
    {
        err("** TEST: %s FAIL %d [%ld ms] **", test_name, ret_val, micros_used);
        err("-------------------------\n");
    }
    else
    {

        ok("** TEST: %s OK [%ld ms] **", test_name, micros_used);
        ok("-------------------------\n");
    }

    if (test_cleanup())
    {
        err("Error in test_cleanup");
        return 1;
    }

    return ret_val;
}
