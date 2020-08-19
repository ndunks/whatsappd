#pragma once

#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "util.h"

#define SENDER_BUF_MAX 10240

typedef enum SENDER_RESULT
{
    SENDER_RESULT_INVALID,
    SENDER_RESULT_PENDING,
    SENDER_RESULT_OK,
    SENDER_RESULT_FAIL,
} SENDER_RESULT;

extern struct SENDER
{
    char *to, *txt;
    SENDER_RESULT result;
    pthread_mutex_t *mutex;
    pthread_cond_t *signal;
} sender;

extern const char *sender_file;
void sender_setup();
int sender_start();
int sender_stop();