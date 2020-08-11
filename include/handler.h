#pragma once
#include "util.h"
#include "chats.h"
#include "binary.h"
#include "wss.h"
#include "wasocket.h"

typedef int (*BINARY_NODE_HANDLE)(BINARY_NODE *);

typedef struct HANDLE
{
    const char *tag;
    BINARY_NODE_HANDLE function;
} HANDLE;

HANDLE *handler_get(const char *tag);
int handler_handle(BINARY_NODE *node);
int handler_preempt();