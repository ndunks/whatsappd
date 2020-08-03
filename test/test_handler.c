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
BINARY_NODE *node;
struct dirent **ent;
int files_idx;

int alphasort_r(const struct dirent **a, const struct dirent **b)
{
    return -strcoll((*a)->d_name, (*b)->d_name);
}

int test_preempt()
{

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

int test_another_preempt()
{
    uint8_t preempt1[] = {
        // first chat count = 1
        // 0xf8, 0x04, 0x4d, 0x5b, 0x13, 0xf8, 0x02, 0xf8, 0x0b, 0x13, 0x2d, 0xfa, 0xff,
        // 0x87, 0x62, 0x85, 0x72, 0x65, 0x01, 0x01, 0x7f, 0x50, 0x18, 0xff, 0x81, 0x1f,
        // 0x57, 0xff, 0x05, 0x15, 0x94, 0x10, 0x09, 0x89, 0x63, 0xff, 0x81, 0x0f, 0x71,
        // 0x20, 0xf8, 0x0b, 0x13, 0x2d, 0xfa, 0xff, 0x87, 0x62, 0x82, 0x31, 0x34, 0x39,
        // 0x39, 0x3f, 0x50, 0x18, 0xff, 0x81, 0x0f, 0x57, 0xff, 0x05, 0x15, 0x94, 0x09,
        // 0x71, 0x27, 0x63, 0xff, 0x81, 0x0f, 0x71, 0x20
        0xf8, 0x04, 0x4d, 0x5b, 0x13, 0xf8, 0x02, 0xf8, 0x0d, 0x13, 0x2d, 0xfa, 0xff,
        0x87, 0x62, 0x85, 0x72, 0x65, 0x01, 0x01, 0x7f, 0x50, 0x18, 0xff, 0x81, 0x2f,
        0x57, 0xff, 0x05, 0x15, 0x96, 0x44, 0x26, 0x45, 0x63, 0xff, 0x81, 0x0f, 0x34,
        0x5a, 0x71, 0x20, 0xf8, 0x0b, 0x13, 0x2d, 0xfa, 0xff, 0x87, 0x62, 0x82, 0x31,
        0x34, 0x39, 0x39, 0x3f, 0x50, 0x18, 0xff, 0x81, 0x0f, 0x57, 0xff, 0x05, 0x15,
        0x94, 0x09, 0x71, 0x27, 0x63, 0xff, 0x81, 0x0f, 0x71, 0x20};

    node = binary_read(preempt1, 73);
    FALSY(node == NULL);

    TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);
    FALSY(node->child.list == NULL);
    info("CHILDS: %d", node->child_len);
    ZERO(handler_handle(node));
    binary_free();
    
    return 0;
}

int test_main()
{
    //return test_preempt() || test_messages() || test_another_preempt();
    return test_another_preempt();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
