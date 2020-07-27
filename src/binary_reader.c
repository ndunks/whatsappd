#include <byteswap.h>
#include <helper.h>

#include "binary_reader.h"

static uint8_t *buf = NULL;
static size_t idx = 0, buf_len = 0;
static int IS_READING_ATTR = 0;

char *read_string_tag(uint32_t bin_tag);

uint8_t read_byte()
{
    return buf[idx++];
}

uint16_t read_int16()
{
    idx += 2;
    return be16toh(*(uint16_t *)&buf[idx - 2]);
}

uint32_t read_int20()
{
    uint32_t value = (buf[idx++] & 0xf) << 16;
    value |= (buf[idx++] & 0xff) << 8;
    value |= (buf[idx++] & 0xff);
    warn("READ int20 %u", value);
    return value;
    // return (uint32_t)(
    //     ((buf[idx++] & 0xf) << 16) |
    //     ((buf[idx++] & 0xff) << 8) |
    //     ((buf[idx++] & 0xff)));
}

uint32_t read_int32()
{
    idx += 4;
    warn("READ int32 %u", (uint32_t *)&buf[idx - 4]);
    return be32toh(*(uint32_t *)&buf[idx - 4]);
}

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
    int i, flag, odd, len;
    flag = read_byte();
    odd = flag >> 7;
    len = 127 & flag;
    str = binary_alloc(len * 2 + 1);
    memset(str, 0, len * 2 + 1);
    accent("read_packed8: %p %u %d", str, tag, len * 2 + 1);

    for (i = 0; i < len; i++)
    {
        flag = read_byte();
        str[i * 2] = unpack_byte(tag, (0xf0 & flag) >> 4);
        str[i * 2 + 1] = unpack_byte(tag, 0x0f & flag);
    }
    info("PACKED: %1$d %2$p %2$s", strlen(str), str);
    if (odd)
    {
        //r = r.substring(0, r.length - 1)
        str[len * 2 - 1] = 0;
    }
    return str;
}

uint read_list_flag(uint8_t bin_tag)
{
    switch (bin_tag)
    {
    case LIST_8:
        return read_byte();
    case LIST_16:
        return read_int16();
    default:
        warn("invalid list size: %d", bin_tag);
    case LIST_EMPTY:
        return 0;
    }
}

const char *read_token(uint8_t no)
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
        str = DICTIONARY_SINGLEBYTE[no];
        if (strcmp(str, wa_host_long) == 0)
            str = wa_host_short;
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
    memcpy(str, &buf[idx], len);
    idx += len;
    return str;
}

char *read_string_tag_jid_pair()
{
    char *user = NULL, *host = NULL, *str = NULL;
    int len;
    user = read_string_tag(read_byte());
    //info("jid_pair %s %s", user, host);
    host = read_string_tag(read_byte());
    //info("jid_pair %p %p", user, host);

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
        agent = read_byte(),
        device = read_byte();
    //user
    len = read_byte();
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
    const char *str;
    accent("read_string_tag: %d", bin_tag);

    if (bin_tag > 2 && bin_tag < 236)
    {
        str = read_token(bin_tag);
        //info("Single token: %s", str);
        return str;
    }
    info("read_str_tag: %d", bin_tag);

    switch (bin_tag)
    {
    case DICTIONARY_0:
    case DICTIONARY_1:
    case DICTIONARY_2:
    case DICTIONARY_3:
        len = read_byte();
        return read_token_double(bin_tag - DICTIONARY_0, len);
    case LIST_EMPTY:
        return NULL;
    case BINARY_8:
        str = read_string_len(read_byte());
        if (IS_READING_ATTR)
            return str;
        else
            return NULL;
    case BINARY_20:
        str = read_string_len(read_int20());
        if (IS_READING_ATTR)
            return str;
        else
            return NULL;

    case BINARY_32:
        str = read_string_len(read_int32());
        if (IS_READING_ATTR)
            return str;
        else
            return NULL;

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

int read_list_size(uint8_t tag)
{
    switch (tag)
    {
    case LIST_EMPTY:
        return 0;
    case LIST_8:
        return read_byte();
    case LIST_16:
        return read_int16();
    default:
        warn("invalid list size. tag %d", tag);
        return 0;
    }
}

void read_attributes(BINARY_NODE *node)
{
    int i;
    BINARY_NODE_ATTR *attr = NULL;
    IS_READING_ATTR = 1;
    //ok("read_attributes: %d", node->attr_len);

    for (i = 0; i < node->attr_len; i++)
    {
        attr = &node->attrs[i];
        //ok("attrs[%d]: %p", i, attr->value);
        attr->key = read_string_tag(read_byte());
        //ok("attrs KEY: %s", attr->key);
        attr->value = read_string_tag(read_byte());
        //ok("attrs[%d]: %p", i, attr->value);

        //accent("  attr %s:%s", attr->key, attr->value);
    }
    IS_READING_ATTR = 0;
}

void read_list(BINARY_NODE *node, uint8_t bin_tag)
{
    int i, len;
    BINARY_NODE **ptr = NULL;
    size_t size;

    len = read_list_size(bin_tag);
    size = sizeof(void *) * (len + 1);

    ok("Read list: %d %d", size, len);
    node->child.list = binary_alloc(size);
    memset(node->child.list, 0, size);
    for (i = 0; i < len; i++)
    {
        //info("CHILD: %p %p", node->child, ptr);
        //ptr = binary_child(node, i);
        ptr = (&node->child.list[i]);
        //info("CHILD: %p %p %p", node->child, ptr, *ptr);
        *ptr = read_node();
        //info("CHILD: %p %p %p", node->child.list[i], ptr, *ptr);
        node->child_len++;
        //info("-------");
        ptr = NULL;
        // if (i == 1)
        //     break;
    }
}

BINARY_NODE *read_node()
{
    uint8_t bin_tag;
    int list_flag;
    BINARY_NODE *node = binary_alloc(sizeof(BINARY_NODE));

    memset(node, 0, sizeof(BINARY_NODE));

    bin_tag = read_byte();
    ok("read_node: %x %d", idx - 1, bin_tag);
    list_flag = read_list_flag(bin_tag);
    bin_tag = read_byte();

    if (bin_tag == STREAM_END)
    {
        err("Unexpected end tag");
        return NULL;
    }
    node->tag = read_string_tag(bin_tag);

    if (list_flag == 0 || node->tag == NULL)
    {
        err("Invalid node");
        return NULL;
    }

    node->attr_len = ((list_flag - 2) + (list_flag % 2)) >> 1;
    info("read_node, tag: %s, list_flag: %d, attr_len: %d", node->tag, list_flag, node->attr_len);
    if (node->attr_len > 0)
    {
        read_attributes(node);
    }

    if ((list_flag % 2) == 1)
    {
        accent("Node nochild");
        node->child_type = BINARY_NODE_CHILD_EMPTY;
        return node;
    }
    ok("node: Read child");

    bin_tag = read_byte();
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
        node->child_len = read_byte();
        node->child.data = read_string_len(node->child_len);
        break;
    case BINARY_20:
        node->child_type = BINARY_NODE_CHILD_BINARY;
        node->child_len = read_int20();
        node->child.data = read_string_len(node->child_len);
        break;
    case BINARY_32:
        node->child_type = BINARY_NODE_CHILD_BINARY;
        node->child_len = read_int32();
        node->child.data = read_string_len(node->child_len);
        break;
    default:
        node->child_type = BINARY_NODE_CHILD_STRING;
        node->child.data = read_string_tag(bin_tag);
        break;
    }

    return node;
}

void binary_read_set(char *src, size_t src_len)
{
    idx = 0;
    buf = (uint8_t *)src;
    buf_len = src_len;
}

BINARY_NODE *binary_read(char *src, size_t src_len)
{
    binary_read_set(src, src_len);
    return read_node();
}
