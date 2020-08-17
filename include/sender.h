#pragma once

#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "util.h"

// #define SENDER_TXT_MAX 1024
// #define SENDER_TO_MAX 32

#define SENDER_BUF_MAX 2048

enum SENDER_RESULT
{
    SENDER_RESULT_INVALID,
    SENDER_RESULT_PENDING,
    SENDER_RESULT_OK,
    SENDER_RESULT_FAIL,
};

extern struct SENDER
{
    char *to, *txt;
    enum SENDER_RESULT result;
    pthread_mutex_t *mutex;
    pthread_cond_t *signal;
} sender;

void sender_setup();
int sender_start();
int sender_stop();