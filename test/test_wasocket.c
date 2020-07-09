#include <malloc.h>
#include "test.h"

static char *buf;

int test_connect()
{
    ASSERT(wasocket_connect());
    return 0;
}

void test_main()
{
    printf("YES RUNNING %s\n", TEST);

    if (test_connect())
        return;

}

int test_setup()
{
    buf = malloc(255);
    return 0;
}

int test_cleanup()
{
    free(buf);
    return 0;
}
