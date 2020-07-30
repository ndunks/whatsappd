#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/file.h>

#include <helper.h>
#include "binary_reader.h"
#include "handler.h"

#include "test.h"

#define BUF_SIZE 80000

static char buf[BUF_SIZE];
static size_t read_size;

int alphasort_r(const struct dirent **a, const struct dirent **b)
{
    return -strcoll((*a)->d_name, (*b)->d_name);
}

int test_main()
{
    struct dirent **ent;
    int files_idx;
    BINARY_NODE *node;

    /** Message sent after preempt is contain chats */
    files_idx = scandir(SAMPLE_DIR, &ent, NULL, alphasort_r);
    while (files_idx--)
    {
        switch (*ent[files_idx]->d_name)
        {
        case '1':
        case 'p':
            break;
        default:
            continue;
        }

        ZERO(load_sample(ent[files_idx]->d_name, buf, BUF_SIZE, &read_size));
        node = binary_read(buf, read_size);
        FALSY(node == NULL);

        TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);
        FALSY(node->child.list == NULL);
        info("CHILDS: %d", node->child_len);
        ZERO(handler_handle(node));

        // for (i = 0; i < node->child_len; i++)
        // {
        //     ptr = node->child.list[i];
        //     info(" %3d: %s", i, ptr->tag);
        // }
        binary_free();
        free(ent[files_idx]);
    }
    free(ent);
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
