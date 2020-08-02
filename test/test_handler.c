#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/file.h>

#include <helper.h>
#include "binary_reader.h"
#include "handler.h"

#include "test.h"

#define BUF_SIZE 80000

static char read_buf[BUF_SIZE];
static size_t read_size;

int alphasort_r(const struct dirent **a, const struct dirent **b)
{
    return -strcoll((*a)->d_name, (*b)->d_name);
}

int test_preempt()
{
    struct dirent **ent;
    int files_idx;
    BINARY_NODE *node;

    TRUTHY(handler_unread_chats == NULL);

    /** Message sent after preempt is contain chats */
    files_idx = scandir(SAMPLE_DIR, &ent, NULL, alphasort_r);
    while (files_idx--)
    {
        switch (*ent[files_idx]->d_name)
        {
        //preempt
        case 'p':
            break;
        default:
            continue;
        }

        ZERO(load_sample(ent[files_idx]->d_name, read_buf, BUF_SIZE, &read_size));
        node = binary_read(read_buf, read_size);
        FALSY(node == NULL);

        TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);
        FALSY(node->child.list == NULL);
        info("CHILDS: %d", node->child_len);
        ZERO(handler_handle(node));

        binary_free();
        free(ent[files_idx]);
    }
    free(ent);
    return 0;
}

int test_messages()
{
    struct dirent **ent;
    int files_idx;
    BINARY_NODE *node;

    TRUTHY(handler_unread_chats == NULL);

    /** Message sent after preempt is contain chats */
    files_idx = scandir(SAMPLE_DIR, &ent, NULL, alphasort_r);
    while (files_idx--)
    {
        switch (*ent[files_idx]->d_name)
        {
        case '1':
            break;
        default:
            continue;
        }

        ZERO(load_sample(ent[files_idx]->d_name, read_buf, BUF_SIZE, &read_size));
        node = binary_read(read_buf, read_size);
        FALSY(node == NULL);

        TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);
        FALSY(node->child.list == NULL);
        info("CHILDS: %d", node->child_len);
        ZERO(handler_handle(node));
        binary_free();
        free(ent[files_idx]);
    }
    ok("Unread chats %lu", handler_unread_count);
    free(ent);
    return 0;
}

int test_main()
{
    return test_preempt() || test_messages();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
