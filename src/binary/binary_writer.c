#include <byteswap.h>
#include <string.h>

#include "binary.h"

size_t write_string(char *str, size_t len)
{
    uint i = 0;

    // check for single bytecode
    if (len > 0 && len < 15)
    {
        for (i = 3; i < 176; i++)
        {
            if (strncmp(str, DICTIONARY_SINGLEBYTE[i], len) == 0)
                return buf_write_byte(i);
        }
    }
    i = 0;
    if (len >= 1 << 20)
    {
        i += buf_write_byte(BINARY_32);
        i += buf_write_int32(len);
    }
    else if (len >= 256)
    {
        i += buf_write_byte(BINARY_20);
        i += buf_write_int20(len);
    }
    else
    {
        i += buf_write_byte(BINARY_8);
        i += buf_write_byte(len);
    }
    return i + buf_write_bytes(str, len);
}

uint write_list_start(uint16_t value)
{
    if (value == 0)
    {
        buf_write_byte(LIST_EMPTY);
        return 1;
    }
    if (value < 256u)
    {
        buf_write_byte(LIST_8);
        buf_write_byte(value & 0xffU);
        return 2;
    }
    else
    {
        buf_write_byte(LIST_16);
        buf_write_int16(value);
        return 3;
    }
}

size_t write_node(BINARY_NODE *node)
{
    uint i;
    size_t size = 0;
    uint16_t flag = 1;

    if (node->attr_len)
        flag += node->attr_len * 2;

    if (node->child_len)
        flag += 1;

    size += write_list_start(flag);
    size += write_string(node->tag, strlen(node->tag));

    // Attributes
    for (i = 0; i < node->attr_len; i++)
    {
        size += write_string(node->attrs[i].key, strlen(node->attrs[i].key));
        size += write_string(node->attrs[i].value, strlen(node->attrs[i].value));
    }

    if (!node->child_len)
        return 0;

    // Children
    switch (node->child_type)
    {
    case BINARY_NODE_CHILD_BINARY:
    case BINARY_NODE_CHILD_STRING:
        size += write_string(node->child.data, node->child_len);
        /* code */
        break;
    case BINARY_NODE_CHILD_LIST:
        size += write_list_start(node->child_len);
        for (i = 0; i < node->child_len; i++)
        {
            size += write_node(node->child.list[i]);
        }
        break;
    default:
        warn("Invalid node list type! %x", node->child_type);
        break;
    }

    return size;
}

size_t binary_write(BINARY_NODE *node, char *dst, size_t dst_len)
{
    buf_set(dst, dst_len);
    return write_node(node);
}