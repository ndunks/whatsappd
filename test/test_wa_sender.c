#include "whatsappd.h"

#include "test.h"
typedef struct TEST_SENDER_ARG
{
    const char *id, *number, **msgs;
} TEST_SENDER_ARG;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t th_handler;

int test_send(const char *number, const char *msg)
{
    int res, f = open(sender_file, O_WRONLY);
    size_t len = strlen(number) + strlen(msg) + 1;
    char *buf = malloc(len);

    if (f < 0)
    {
        warn("Fail open: %s", sender_file);
        return -1;
    }
    len = sprintf(buf, "%s\n%s", number, msg);
    if (write(f, buf, len) < len)
    {
        warn("Not fully write");
        return -1;
    }
    close(f);
    f = open(sender_file, O_RDONLY);
    if (f < 0)
    {
        warn("Fail open: %s", sender_file);
        return -1;
    }
    len = read(f, buf, 1);
    if (len < 1)
    {
        warn("Read fail");
        return -1;
    }
    close(f);
    res = buf[0] - 48;
    ok("SEND RESULT: %d", res);
    free(buf);
    return res != SENDER_RESULT_OK;
}

int *test_sender(TEST_SENDER_ARG *arg)
{
    char **ptr = (char **)arg->msgs;
    int *ret = malloc(sizeof(int)), total = 0;
    //info("test_sender start %s", arg->id);
    while (*ptr != NULL)
    {
        pthread_mutex_lock(&mutex);
        *ret = test_send(arg->number, *ptr);
        pthread_mutex_unlock(&mutex);
        sleep(0);
        accent("%s: %d %d", arg->id, total, *ret);
        if (*ret == 0)
            total++;
        ptr++;
    }
    warn("%s: QUIT %d", arg->id, total);
    *ret = total;
    return ret;
}

int test_main()
{

    const char *number = "6285726501017",
               *msgs1[] = {
                   "TEST A",
                   "TEST B",
                   "TEST C",
                   NULL,
               },
               *msgs2[] = {
                   "MSG A",
                   "MSG B",
                   "MSG C",
                   NULL,
               };
    TEST_SENDER_ARG arg1 = {.id = "THREAD-1", .number = number, .msgs = msgs1},
                    arg2 = {.id = "THREAD-2", .number = number, .msgs = msgs2},
                    arg3 = {.id = "THREAD-3", .number = "0", .msgs = msgs2};
    pthread_t th1, th2, th3;
    int *ret1, *ret2, *ret3;

    ZERO(access(sender_file, R_OK | W_OK));

    ZERO(pthread_create(&th1, NULL, (void *(*)(void *))test_sender, (void *)&arg1));
    ZERO(pthread_create(&th2, NULL, (void *(*)(void *))test_sender, (void *)&arg2));
    ZERO(pthread_create(&th3, NULL, (void *(*)(void *))test_sender, (void *)&arg3));

    pthread_join(th1, (void **)&ret1);
    pthread_join(th2, (void **)&ret2);
    pthread_join(th3, (void **)&ret3);
    TRUTHY(*ret1 == 3);
    TRUTHY(*ret2 == 3);
    TRUTHY(*ret3 == 0);

    free(ret1);
    free(ret2);
    free(ret3);
    return 0;
}

int test_setup()
{
    ZERO(whatsappd_init(NULL));
    ZERO(pthread_create(&th_handler, NULL, (void *(*)(void *))whatsappd_autoreply, NULL));
    sleep(1);
    ok("test setup OK");
    return 0;
}

int test_cleanup()
{
    whatsappd_free();
    pthread_join(th_handler, NULL);
    return 0;
}
