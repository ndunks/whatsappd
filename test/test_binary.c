#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/file.h>

#include "util.h"
#include "binary.h"

#include "test.h"

#define BUF_SIZE 80000

static char read_buf[BUF_SIZE];
static size_t read_size;

int alphasort_r(const struct dirent **a, const struct dirent **b)
{
    return -strcoll((*a)->d_name, (*b)->d_name);
}

int test_buf()
{
    char local_buf[] = "\xcd"              // int8
                 "\xf0\x0f"          // int16
                 "\x0a\xbb\x0c"      // int 20
                 "\xaa\xbb\xcc\xdd"; // int32
    buf_set(local_buf, 9);
    //accent("int8  0x%02x", buf_read_byte());
    //accent("int16 0x%04x", buf_read_int16());
    //accent("int20 0x%06x", buf_read_int20());
    //accent("int32 0x%08x", buf_read_int32());

    // reset index, keep old data
    buf_set(local_buf, 9);

    TRUTHY(buf_read_byte() == 0xcd);
    TRUTHY(buf_read_int16() == 0xf00f);
    TRUTHY(buf_read_int20() == 0x0abb0c);
    TRUTHY(buf_read_int32() == 0xaabbccdd);
    // TRUTHY(read_int16() == 0x0ff0);
    // TRUTHY(read_int20() == 0x0cbbaa);
    // TRUTHY(read_int32() == 0xaabbccdd);
    return 0;
}

int test_preempt()
{
    int i;
    BINARY_NODE *ptr, *node;

    // First preempts is contact lists
    ZERO(load_sample("preempt-1589472058-318", read_buf, BUF_SIZE, &read_size));
    node = binary_read(read_buf, read_size);
    FALSY(node == NULL);

    ZERO(strcmp(node->tag, "response"));
    TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);
    TRUTHY(node->child_len == 901);

    TRUTHY(node->attr_len == 2);
    FALSY(node->attrs[0].key == NULL);
    FALSY(node->attrs[1].key == NULL);
    TRUTHY(node->attrs[2].key == NULL);
    FALSY(binary_attr(node, "checksum") == NULL);
    FALSY(binary_attr(node, "type") == NULL);
    ZERO(strcmp(binary_attr(node, "checksum"), "1BE33034-E32C-40EA-BB66-554B81AF0AE7"));
    ZERO(strcmp(binary_attr(node, "type"), "contacts"));

    for (i = 0; i < node->child_len; i++)
    {
        ptr = binary_child(node, i);
        TRUTHY(ptr != NULL);
        ZERO(strcmp(ptr->tag, "user"));
    }

    ptr = binary_child(node, 14);
    ZERO(ptr->child_len);
    TRUTHY(ptr->child.list == NULL);
    FALSY(binary_attr(ptr, "notify") == NULL);
    FALSY(binary_attr(ptr, "verify") == NULL);
    FALSY(binary_attr(ptr, "vname") == NULL);
    FALSY(binary_attr(ptr, "jid") == NULL);
    ZERO(strcmp(binary_attr(ptr, "notify"), "defri reza"));
    ZERO(strcmp(binary_attr(ptr, "verify"), "0"));
    ZERO(strcmp(binary_attr(ptr, "vname"), "defri reza"));
    ZERO(strcmp(binary_attr(ptr, "jid"), "62816655404@c.us"));

    binary_alloc_stat();
    binary_free();

    // second preempt status?
    ZERO(load_sample("preempt-1589472058-319", read_buf, BUF_SIZE, &read_size));
    node = binary_read(read_buf, read_size);
    FALSY(node == NULL);

    ZERO(strcmp(node->tag, "response"));
    TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);
    TRUTHY(node->child_len == 292);

    FALSY(binary_attr(node, "status") == NULL);
    FALSY(binary_attr(node, "type") == NULL);
    ZERO(strcmp(binary_attr(node, "status"), "992971"));
    ZERO(strcmp(binary_attr(node, "type"), "chat"));

    for (i = 0; i < node->child_len; i++)
    {
        ptr = binary_child(node, i);
        TRUTHY(ptr != NULL);
        ZERO(strcmp(ptr->tag, "chat"));
    }

    ptr = binary_child(node, 2);
    FALSY(binary_attr(ptr, "t") == NULL);
    FALSY(binary_attr(ptr, "count") == NULL);
    FALSY(binary_attr(ptr, "spam") == NULL);
    FALSY(binary_attr(ptr, "jid") == NULL);
    FALSY(binary_attr(ptr, "modify_tag") == NULL);
    FALSY(binary_attr(ptr, "name") == NULL);
    ZERO(strcmp(binary_attr(ptr, "t"), "1588824994"));
    ZERO(strcmp(binary_attr(ptr, "count"), "0"));
    ZERO(strcmp(binary_attr(ptr, "spam"), "false"));
    ZERO(strcmp(binary_attr(ptr, "jid"), "6281230008708@c.us"));
    ZERO(strcmp(binary_attr(ptr, "modify_tag"), "406232"));
    ZERO(strcmp(binary_attr(ptr, "name"), "Ade"));
    return 0;
}

int test_read_message()
{
    struct dirent **ent;
    int files_idx, i;
    BINARY_NODE *ptr, *node;

    /** Message sent after preempt is contain chats */
    files_idx = scandir(SAMPLE_DIR, &ent, NULL, alphasort_r);
    while (files_idx--)
    {
        if (*ent[files_idx]->d_name != '1')
            continue;

        ZERO(load_sample(ent[files_idx]->d_name, read_buf, BUF_SIZE, &read_size));
        node = binary_read(read_buf, read_size);
        FALSY(node == NULL);

        ZERO(strcmp(node->tag, "action"));
        TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);
        FALSY(node->child.list == NULL);
        //info("CHILDS: %d", node->child_len);

        for (i = 0; i < node->child_len; i++)
        {
            ptr = node->child.list[i];
            //info(" %3d: %s", i, ptr->tag);
        }
        binary_free();
        free(ent[files_idx]);
    }
    free(ent);
    return 0;
}

int test_main()
{
    return test_buf() || test_preempt() || test_read_message();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
