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
    // buf_set(local_buf, 9);
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

int test_read_write()
{
    char binbuf[256];
    size_t len;
    BINARY_NODE node, *node2;
    node.tag = "response";
    node.attr_len = 2;
    node.attrs[0].key = "recent";
    node.attrs[0].value = "200";
    node.attrs[1].key = "hello";
    node.attrs[1].value = "world";
    node.child_len = 4;
    node.child_type = BINARY_NODE_CHILD_BINARY;
    node.child.data = "ABCD";

    len = binary_write(&node, binbuf, 256);
    hexdump(binbuf, len + 2);
    node2 = binary_read(binbuf, len);
    ZERO(strcmp(node.tag, node2->tag));
    TRUTHY(node.attr_len == node2->attr_len);
    TRUTHY(node.child_len == node2->child_len);
    TRUTHY(node.child_type == node2->child_type);

    ZERO(strcmp(node.attrs[0].key, node2->attrs[0].key));
    ZERO(strcmp(node.attrs[0].value, node2->attrs[0].value));
    ZERO(strcmp(node.attrs[1].key, node2->attrs[1].key));
    ZERO(strcmp(node.attrs[1].value, node2->attrs[1].value));
    ZERO(strcmp(node.child.data, node2->child.data));

    return 0;
}

int test_real_message()
{
    struct dirent **ent;
    int files_idx, i;
    char re_build[80000] = {0};
    BINARY_NODE *ptr, *ptr2, *node, *node2;
    size_t re_build_size;

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

        re_build_size = binary_write(node, re_build, 80000);
        //info(" rebuild %lu vs %lu", read_size, re_build_size);
        TRUTHY(re_build_size >= read_size);

        node2 = binary_read(re_build, re_build_size);
        FALSY(node2 == NULL);
        ZERO(strcmp(node->tag, node2->tag));

        // info("ATTRS : %d vs %d", node->attr_len, node2->attr_len);
        // info("CHILDS: %d vs %d", node->child_len, node2->child_len);
        TRUTHY(node->attr_len == node2->attr_len);
        TRUTHY(node->child_len == node2->child_len);
        for (i = 0; i < node->attr_len; i++)
        {
            ZERO(strcmp(node->attrs[i].key, node2->attrs[i].key));
            ZERO(strcmp(node->attrs[i].value, node2->attrs[i].value));
        }

        for (i = 0; i < node->child_len; i++)
        {
            ptr = node->child.list[i];
            ptr2 = node2->child.list[i];
            TRUTHY(ptr->attr_len == ptr2->attr_len);
            TRUTHY(ptr->child_len == ptr2->child_len);
            TRUTHY(ptr->child_type == ptr2->child_type);
            ZERO(strcmp(ptr->tag, ptr2->tag));
        }

        binary_free();
        free(ent[files_idx]);
    }
    free(ent);
    return 0;
}

int test_main()
{
    return test_buf() || test_preempt() || test_read_write() || test_real_message();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
