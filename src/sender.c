#include "sender.h"

const char *sender_file = "/dev/shm/whatsappd";
static char *sender_buf;

static pthread_t sender_thread;
struct SENDER sender;

void *sender_main()
{
    int f;
    ssize_t recv, total, separator_len;
    char *separator, reply[1];
    while (1)
    {
        sender_buf[0] = 0;
        separator = NULL;
        total = 0;
        f = open(sender_file, O_RDONLY);
        //info("sender: wait");
        if (f < 0)
        {
            err("sender: Fail OPEN R");
            sleep(1);
            continue;
        }
        do
        {
            if (total + 1 >= SENDER_BUF_MAX)
            {
                close(f);
                pthread_mutex_lock(sender.mutex);
                warn("sender: Too large!");
                goto REPLY;
            }
            recv = read(f, sender_buf + total, SENDER_BUF_MAX - total);
            //err("sender: read: %ld", recv);
            if (separator == NULL)
            {
                for (separator_len = total; separator_len < total + recv; separator_len++)
                {
                    // all non numeric will treat as separator
                    if (sender_buf[separator_len] < '0' || sender_buf[separator_len] > '9')
                    {
                        separator = sender_buf + separator_len;
                        *separator = 0;
                        break;
                    }
                }
            }
            total += recv;
        } while (recv > 0);
        close(f);
        sender_buf[total] = 0;

        pthread_mutex_lock(sender.mutex);
        if (separator == NULL)
        {
            // is invalid
            warn("sender: No line separator");
            goto REPLY;
        }

        if (separator_len < 4)
        {
            warn("sender: Number too short");
            goto REPLY;
        }

        if (separator_len > 24)
        {
            warn("sender: Number too long");
            goto REPLY;
        }

        if (total == separator_len)
        {
            warn("sender: Message too short");
            goto REPLY;
        }

        sender.result = SENDER_RESULT_PENDING;
        sender.txt = separator + 1;
        //info("sender: %s\n%s", sender.to, sender.txt);
        while (sender.result == SENDER_RESULT_PENDING)
        {
            //info("sender: waiting reply");
            pthread_cond_wait(sender.signal, sender.mutex);
        }
    REPLY:
        //ok("sender: %d", sender.result);
        f = open(sender_file, O_WRONLY);
        if (f < 0)
        {
            err("sender: Fail OPEN W");
            continue;
        }
        reply[0] = sender.result + '0';
        write(f, reply, 1);
        close(f);
        sender.result = SENDER_RESULT_INVALID;
        pthread_mutex_unlock(sender.mutex);
    }
    warn("sender main EXITED");
    return NULL;
}

void sender_setup()
{
    memset(&sender, 0, sizeof(struct SENDER));
    sender.mutex = calloc(sizeof(pthread_mutex_t), 1);
    sender.signal = calloc(sizeof(pthread_cond_t), 1);
    sender_buf = malloc(SENDER_BUF_MAX);
    sender.to = sender_buf;
}

int sender_start()
{
    CHECK(pthread_mutex_init(sender.mutex, NULL));
    if (access(sender_file, F_OK) == 0)
    {
        // sender_file exists and has RW Access
        if (access(sender_file, W_OK) != 0)
        {
            err("sender: Can't access %s", sender_file);
            return -1;
        }
        CHECK(unlink(sender_file));
    }
    CHECK(mkfifo(sender_file, 0666));
    return pthread_create(&sender_thread, NULL, sender_main, NULL);
}

int sender_stop()
{
    CHECK(pthread_cancel(sender_thread));
    CHECK(pthread_join(sender_thread, NULL));
    CHECK(pthread_mutex_destroy(sender.mutex));
    info("sender stopped");
    free(sender.mutex);
    free(sender.signal);
    free(sender_buf);
    unlink(sender_file);
    return 0;
}
