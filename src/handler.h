#pragma once

typedef int (*BINARY_NODE_HANDLE)(BINARY_NODE *);

typedef struct HANDLE
{
    const char *tag;
    BINARY_NODE_HANDLE function;

} HANDLE;

typedef struct UNREAD_MSG
{
    // Phone number converted to number for fast searching
    uint64_t id;
    char *jid, *name, *msg;
    struct UNREAD_MSG *next;
} UNREAD_MSG;

size_t handler_unread_count;
UNREAD_MSG handler_undread;

HANDLE *handler_get(const char *tag);
int handler_handle(BINARY_NODE *node);