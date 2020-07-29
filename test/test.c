#include "test.h"
//#include <sys/time.h>
#include <time.h>
#define NANO_PER_SEC 1000000000.0

int ret_val;
int main(int argc, char const *argv[])
{
    char test_name[] = TEST;
    struct timespec start, end;
    double start_sec, end_sec, elapsed_sec;
    //long secs_used, micros_used;

    info("** TEST: %s **", test_name);

    if (test_setup())
    {
        err("Error in test_setup");
        return 1;
    }

    clock_gettime(CLOCK_REALTIME, &start);
    ret_val = test_main();
    clock_gettime(CLOCK_REALTIME, &end);
    start_sec = start.tv_sec + start.tv_nsec / NANO_PER_SEC;
    end_sec = end.tv_sec + end.tv_nsec / NANO_PER_SEC;
    elapsed_sec = end_sec - start_sec;
    // secs_used = (end.tv_sec - start.tv_sec); //avoid overflow by subtracting first
    // micros_used = ((secs_used * 1000000) + end.tv_usec) - (start.tv_usec);

    if (ret_val != 0)
    {
        err("** TEST: %s FAIL %d [%.4f ms] **", test_name, ret_val, elapsed_sec);
        err("-------------------------\n");
    }
    else
    {

        ok("** TEST: %s OK [%.4f ms] **", test_name, elapsed_sec);
        ok("-------------------------\n");
    }

    if (test_cleanup())
    {
        err("Error in test_cleanup");
        return 1;
    }

    return ret_val;
}
