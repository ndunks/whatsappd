#include "sender.h"

#include "test.h"

int test_main()
{
    int counter = 0;
    sender_setup();
    ZERO(sender_start());
    while (1)
    {
        sleep(1);
        pthread_mutex_lock(sender.mutex);
        if (sender.result == SENDER_RESULT_PENDING)
        {
            info("Got pending\n%s\n%s", sender.to, sender.txt);
            sender.result = SENDER_RESULT_OK;
            pthread_cond_signal(sender.signal);
        }
        pthread_mutex_unlock(sender.mutex);
        if (counter++ > 30)
        {
            ZERO(sender_stop());
            break;
        }
    }
    return 0;
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
