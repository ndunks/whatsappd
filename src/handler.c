#include <helper.h>
#include <binary.h>

#include "handler.h"
typedef int (*BINARY_NODE_HANDLE)(BINARY_NODE *);

typedef struct HANDLE
{
    const char *tag;
    BINARY_NODE_HANDLE handle;

} HANDLE;

static int handle_action(BINARY_NODE *node)
{
    accent("handle_action: %s", node->tag);
    binary_print_attr(node);
    for (int i = 0; i < node->child_len; i++)
    {
        handler_handle(node->child.list[i]);
    }
    return 0;
}

static int handle_response(BINARY_NODE *node)
{
    accent("handle_response: %s", node->tag);
    return 0;
}

static int handle_message(BINARY_NODE *node)
{
    accent("handle_message: %s", node->tag);
    // if (node->attr_len > 0)
    //     binary_print_attr(node);
    return 0;
}

static int handle_user(BINARY_NODE *node)
{
    accent("handle_user: %s", node->tag);
    return 0;
}

HANDLE handler_tag[] = {
    {.tag = "action", .handle = handle_action},
    {.tag = "response", .handle = handle_response},
    {.tag = "message", .handle = handle_message},
    {.tag = "user", .handle = handle_user},
    NULL};

int handler_handle(BINARY_NODE *node)
{
    const HANDLE *ptr;
    int tag_len;
    const char *tag = node->tag;
    tag_len = strlen(tag);

    if (tag_len == 0)
    {
        warn("handler tag too short");
        return 1;
    }

    ptr = &handler_tag[0];
    do
    {
        if (strncmp(tag, ptr->tag, tag_len))
            continue;
        return (*ptr->handle)(node);

    } while ((++ptr) != NULL);
    warn("Unhandled: %s", tag);
    return 1;
}