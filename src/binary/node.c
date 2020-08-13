#include "binary.h"

BINARY_NODE *binary_node_action(char *type, BINARY_NODE *child)
{
    BINARY_NODE *node;
    node = malloc(sizeof(BINARY_NODE));
    node->tag = "action";
    node->attrs[0].key = "type";
    node->attrs[0].value = type;
    node->attrs[1].key = "epoch";
    node->attrs[1].value = helper_epoch();
    node->attr_len = 2;

    node->child_type = BINARY_NODE_CHILD_LIST;
    node->child.list = malloc(sizeof(void *));
    node->child.list[0] = child;
    node->child_len = 1;
    return node;
}

// Not freeing the childs if is a NODE_LIST
void binary_node_action_free(BINARY_NODE *node)
{
    void *ptr_child = (void *)node->child.data;
    if (ptr_child != NULL)
    {
        free(ptr_child);
    }
    free(node);
}
