#include <byteswap.h>

#include "binary.h"

char *read_string_tag(uint32_t bin_tag);

char unpack_nibble(int e)
{
    if (e < 0 || e > 15)
    {
        warn("unpack_nibble: invalid %d", e);
        return 'X';
    }

    /* nibbleDecode */
    if (e >= 0 && e <= 9)
        return 48 + e;

    switch (e)
    {
    case 10:
        return '-';
    case 11:
        return '.';
    case 15:
        return 0;
    }
    return 'X';
}

char unpack_hex(int e)
{
    if (e < 0 || e > 15)
    {
        warn("unpack_hex: invalid %d", e);
        return 0;
    }
    if (e < 10)
        return 48 + e;
    else
        return 87 + e;
}

char unpack_byte(uint8_t tag, int val)
{
    switch (tag)
    {
    case NIBBLE_8:
        return unpack_nibble(val);
    case HEX_8:
        return unpack_hex(val);
    default:
        warn("unpack non-nibble/hex type: %u", tag);
        return '*';
        break;
    }
}

char *read_packed8(uint8_t tag)
{
    char *str;
    int i, flag, len;

    flag = buf_read_byte();
    len = 127 & flag;
    str = binary_alloc(len * 2 + 1);
    memset(str, 0, len * 2 + 1);

    for (i = 0; i < len; i++)
    {
        flag = buf_read_byte();
        str[i * 2] = unpack_byte(tag, (0xf0 & flag) >> 4);
        str[i * 2 + 1] = unpack_byte(tag, 0x0f & flag);
    }

    return str;
}

char *read_token(uint8_t no)
{
    char *str;

    if (no < 3 || no >= DICTIONARY_SINGLEBYTE_LEN)
    {
        // if WhatsApp updated the list, we may miss it.
        str = binary_alloc(16);
        sprintf(str, "TOKEN_%u", no);
    }
    else
    {
        // read only!
        str = (char *)DICTIONARY_SINGLEBYTE[no];
        if (strcmp(str, wa_host_long) == 0)
            str = (char *)wa_host_short;
    }
    return str;
}

char *read_token_double(uint8_t no, uint8_t len)
{
    char *a = binary_alloc(30);
    sprintf(a, "doublebyte_%u_%u", no, len);
    return a;
}

char *read_string_len(int len)
{
    char *str = binary_alloc(len);
    memcpy(str, &buf[buf_idx], len);
    buf_idx += len;
    return str;
}

char *read_string_tag_jid_pair()
{
    char *user = NULL, *host = NULL, *str = NULL;
    int len;

    user = read_string_tag(buf_read_byte());
    host = read_string_tag(buf_read_byte());

    if (user != NULL && host != NULL)
    {
        len = strlen(user) + strlen(host) + 2;
        str = binary_alloc(len);
        sprintf(str, "%s@%s", user, host);
        return str;
    }

    if (NULL != host)
        return host;

    warn("invalid jid");
    return NULL;
}

char *read_string_tag_jid_ad()
{
    char *user, *str;
    uint8_t len,
        agent = buf_read_byte(),
        device = buf_read_byte();
    //user
    len = buf_read_byte();
    user = read_string_tag(len);

    if (user == NULL)
    {
        warn("invalid JID_AD");
        return NULL;
    }
    if (!agent && !device)
        return user;

    str = binary_alloc(len + 10);

    if (agent)
    {
        if (device)
            sprintf(str, "%s.%d:%d", user, agent, device);
        else
            sprintf(str, "%s.%d", user, agent);
    }
    else if (device)
        sprintf(str, "%s:%d", user, device);

    return str;
}

char *read_string_tag(uint32_t bin_tag)
{
    int len;
    char *str;

    if (bin_tag > 2 && bin_tag < 236)
    {
        str = read_token(bin_tag);
        return str;
    }

    if (!buf_available())
    {
        warn("read_string_tag EOF");
        return NULL;
    }

    switch (bin_tag)
    {
    case DICTIONARY_0:
    case DICTIONARY_1:
    case DICTIONARY_2:
    case DICTIONARY_3:
        len = buf_read_byte();
        return read_token_double(bin_tag - DICTIONARY_0, len);

    case LIST_EMPTY:
        return NULL;

    case BINARY_8:
        str = read_string_len(buf_read_byte());
        return str;

    case BINARY_20:
        str = read_string_len(buf_read_int20());
        return str;

    case BINARY_32:
        str = read_string_len(buf_read_int32());
        return str;

    case JID_PAIR:
        return read_string_tag_jid_pair();

    case JID_AD:
        return read_string_tag_jid_ad();

    case NIBBLE_8:
    case HEX_8:
        return read_packed8(bin_tag);

    default:
        warn("invalid string tag");
        return NULL;
    }
}

void read_attributes(BINARY_NODE *node)
{
    int i;
    BINARY_NODE_ATTR *attr = NULL;

    for (i = 0; i < node->attr_len; i++)
    {
        if (!buf_available())
        {
            warn("read_attributes: EOF");
            break;
        }
        attr = &node->attrs[i];
        attr->key = read_string_tag(buf_read_byte());
        attr->value = read_string_tag(buf_read_byte());
    }
}

void read_list(BINARY_NODE *node, uint8_t bin_tag)
{
    int i, len;
    BINARY_NODE **ptr = NULL;
    size_t size;

    switch (bin_tag)
    {
    case LIST_EMPTY:
        return;
    case LIST_8:
        len = buf_read_byte();
        break;
    case LIST_16:
        len = buf_read_int16();
        break;
    default:
        warn("invalid list size. tag %d", bin_tag);
        return;
    }

    size = sizeof(void *) * (len + 1);
    node->child.list = binary_alloc(size);
    memset(node->child.list, 0, size);

    for (i = 0; i < len; i++)
    {
        if (!buf_available())
        {
            warn("read_list: EOF");
            break;
        }
        ptr = &node->child.list[i];
        *ptr = read_node();
        node->child_len++;
        ptr = NULL;
    }
}

BINARY_NODE *read_node()
{
    uint8_t bin_tag;
    int list_flag;
    BINARY_NODE *node;

    bin_tag = buf_read_byte();

    if (!buf_available())
    {
        warn("read_node: EOF");
        return NULL;
    }

    switch (bin_tag)
    {
    case LIST_8:
        list_flag = buf_read_byte();
        break;
    case LIST_16:
        list_flag = buf_read_int16();
        break;
    case LIST_EMPTY:
        list_flag = 0;
        break;
    default:
        warn("invalid list size: %d", bin_tag);
        return NULL;
    }

    bin_tag = buf_read_byte();

    node = binary_alloc(sizeof(BINARY_NODE));
    memset(node, 0, sizeof(BINARY_NODE));
    node->tag = read_string_tag(bin_tag);

    if (list_flag == 0 || node->tag == NULL)
    {
        err("Invalid node, size %lu", buf_len);
        if (buf_len < 256)
        {
            hexdump(buf, buf_len);
            warn("-------------------");
            fwrite(buf, 1, buf_len, stderr);
            warn("\n-------------------");
        }
        return NULL;
    }

    node->attr_len = ((list_flag - 2) + (list_flag % 2)) >> 1;
    if (node->attr_len > 0)
        read_attributes(node);

    if ((list_flag % 2) == 1)
    {
        node->child_type = BINARY_NODE_CHILD_EMPTY;
        return node;
    }

    bin_tag = buf_read_byte();
    switch (bin_tag)
    {
    case LIST_EMPTY:
    case LIST_8:
    case LIST_16:
        node->child_type = BINARY_NODE_CHILD_LIST;
        read_list(node, bin_tag);
        break;
    case BINARY_8:
        node->child_type = BINARY_NODE_CHILD_BINARY;
        node->child_len = buf_read_byte();
        node->child.data = read_string_len(node->child_len);
        break;
    case BINARY_20:
        node->child_type = BINARY_NODE_CHILD_BINARY;
        node->child_len = buf_read_int20();
        node->child.data = read_string_len(node->child_len);
        break;
    case BINARY_32:
        node->child_type = BINARY_NODE_CHILD_BINARY;
        node->child_len = buf_read_int32();
        node->child.data = read_string_len(node->child_len);
        break;
    default:
        node->child_type = BINARY_NODE_CHILD_STRING;
        node->child.data = read_string_tag(bin_tag);
        break;
    }

    return node;
}

BINARY_NODE *binary_read(char *src, size_t src_len)
{
    buf_set(src, src_len);
    return read_node();
}
