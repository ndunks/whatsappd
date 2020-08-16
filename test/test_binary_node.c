#include "crypto.h"
#include "binary.h"

#include "test.h"
int test_real_presence()
{
    BINARY_NODE *node, *child;
    int i;
    uint8_t node_buf[] = {248, 6, 9, 91, 82, 107, 252, 1, 50, 248, 1, 248, 3, 65, 91, 93};

    node = binary_read((char *)node_buf, sizeof(node_buf));
    binary_alloc_stat();
    TRUTHY(node != NULL);
    TRUTHY(node->child_len == 1);
    TRUTHY(node->child_type == BINARY_NODE_CHILD_LIST);

    for (i = 0; i < node->attr_len; i++)
    {
        TRUTHY(node->attrs[i].key != NULL);
        TRUTHY(node->attrs[i].value != NULL);
    }

    child = node->child.list[0];

    for (i = 0; i < child->attr_len; i++)
    {
        TRUTHY(child->attrs[i].key != NULL);
        TRUTHY(child->attrs[i].value != NULL);
    }

    binary_free();
    binary_alloc_stat();
    ok("test_real_presence OK");
    return 0;
}

int test_rw()
{
    BINARY_NODE *node, child, *node2, *child2;
    BINARY_NODE_ATTR *attr;
    size_t node_size;
    char *node_buf = malloc(256), available;
    int i;

    //memset(&node, 0, sizeof(BINARY_NODE));
    memset(&child, 0, sizeof(BINARY_NODE));

    child.tag = "presence";
    attr = &child.attrs[child.attr_len++];
    attr->key = "type";
    crypto_random(&available, 1);
    available = available % 1;
    attr->value = available ? "available" : "unavailable";

    node = binary_node_action("set", &child);

    binary_alloc_stat();
    node_size = binary_write(node, node_buf, 256);
    binary_alloc_stat();

    TRUTHY(node_size);
    node2 = binary_read(node_buf, node_size);
    binary_alloc_stat();
    TRUTHY(node2 != NULL);
    TRUTHY(node2->attr_len == node->attr_len);
    
    for (i = 0; i < node->attr_len; i++)
    {
        ZERO(strcmp(node2->attrs[i].key, node->attrs[i].key));
        ZERO(strcmp(node2->attrs[i].value, node->attrs[i].value));
    }

    TRUTHY(node2->child_len == node->child_len);
    TRUTHY(node2->child_type == node->child_type);
    child2 = node2->child.list[0];

    TRUTHY(child2 != NULL);
    TRUTHY(child2->attr_len == child.attr_len);

    for (i = 0; i < child.attr_len; i++)
    {
        ZERO(strcmp(child2->attrs[i].key, child.attrs[i].key));
        ZERO(strcmp(child2->attrs[i].value, child.attrs[i].value));
    }

    TRUTHY(child2->child_len == child.child_len);
    TRUTHY(child2->child_type == child.child_type);
    binary_node_action_free(node);
    binary_free();
    free(node_buf);
    binary_alloc_stat();
    return 0;
}

int test_main()
{
    return test_real_presence() || test_rw();
}
int test_setup()
{
    crypto_init();
    return 0;
}

int test_cleanup()
{
    crypto_free();
    return 0;
}
