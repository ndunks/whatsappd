#include <byteswap.h>
#include <helper.h>

#include "binary_reader.h"
#define BINARY_MALLOC_MAX 256

static uint8_t *buf = NULL;
static size_t len, idx;
static int READING_ATTR_FLAG = 0, malloc_idx = 0;
static void *malloc_stacks[BINARY_MALLOC_MAX] = {0};

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
    return (uint32_t)(
        ((buf[idx++] & 0xf) << 16) |
        ((buf[idx++] & 0xff) << 8) |
        ((buf[idx++] & 0xff)));
}

uint32_t read_int32()
{
    idx += 4;
    return be32toh(*(uint32_t *)&buf[idx - 4]);
}
char unpack_nibble(int e)
{
    if (e < 0 || e > 15)
    {
        warn("unpack_nibble: invalid %d", e);
        return 0;
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

void unpack_byte(char *buf, uint8_t tag, int val)
{
    switch (tag)
    {
    case NIBBLE_8:
        *buf = unpack_nibble(val);
        break;
    case HEX_8:
        *buf = unpack_hex(val);
        break;
    default:
        warn("unpack non-nibble/hex type: %u", tag);
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
    str = binary_alloc(len * 2);

    accent("read_packed8: %u", tag);

    for (i = 0; i < len; i += 2)
    {
        flag = read_byte();
        unpack_byte(str + i, tag, (0xf0 & flag) >> 4);
        unpack_byte(str + i + 1, tag, 0x0f & flag);
    }
    if (odd)
    {
        //r = r.substring(0, r.length - 1)
        str[len * 2 - 1] = 0;
    }

    return str;
}

uint read_list_size(uint8_t bin_tag)
{
    switch (bin_tag)
    {
    case LIST_8:
        return read_byte();
    case LIST_16:
        return read_int16();
    default:
        warn("invalid list size");
    case LIST_EMPTY:
        return 0;
    }
}

char *read_token(uint8_t no)
{
    char *buf;

    if (no < 3 || no >= DICTIONARY_SINGLEBYTE_LEN)
    {
        buf = binary_alloc(24);
        sprintf(buf, "single_byte_%u", no);
        return buf;
    }
    else
    {
        return DICTIONARY_SINGLEBYTE[no];
    }
}

char *read_token_double(uint8_t no, uint8_t len)
{
    char *a = binary_alloc(30);
    sprintf(a, "doublebyte_%u_%u", no, len);
    return a;
}

char *read_string_len(int len)
{
    char *str = binary_alloc(len + 1);
    memcpy(str, buf[idx], len);
    str[len + 1] = 0;
    idx += len;
    return str;
}

char *read_string_tag_jid_pair()
{
    char *user, *host, *str;
    int len;
    user = read_string_tag(read_byte());
    host = read_string_tag(read_byte());
    if (NULL != user && NULL != host)
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
    char *str;

    if (bin_tag > 2 && bin_tag < 236)
    {
        str = read_token(bin_tag);
        if (strcmp(str, "s.whatsapp.net") == 0)
            strcpy(str, "c.us");
        return str;
    }

    switch (bin_tag)
    {
    case DICTIONARY_0:
    case DICTIONARY_1:
    case DICTIONARY_2:
    case DICTIONARY_3:
        len = read_byte();
        return read_token_double(bin_tag - DICTIONARY_0, len);
    case LIST_EMPTY:
        return;
    case BINARY_8:
        str = read_string_len(read_byte());
        if (READING_ATTR_FLAG)
            return str;
        else
            return NULL;
    case BINARY_20:
        str = read_string_len(read_int20());
        if (READING_ATTR_FLAG)
            return str;
        else
            return NULL;

    case BINARY_32:
        str = read_string_len(read_int32());
        if (READING_ATTR_FLAG)
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

int read_node()
{
    uint8_t bin_tag;
    uint listSize;
    char *str_tag;

    bin_tag = read_byte();
    listSize = read_list_size(bin_tag);
    bin_tag = read_byte();

    if (bin_tag == STREAM_END)
    {
        err("Unexpected end tag");
        return 1;
    }
    str_tag = read_string_tag(bin_tag);
    info("read_node, tag: %s, listSize: %lu", str_tag, listSize);
}

void binary_read_set(char *src, size_t src_len)
{
    idx = 0;
    buf = (uint8_t *)src;
    len = src_len;
}

int binary_read(char *src, size_t src_len)
{
    binary_read_set(src, src_len);
    return read_node();
}

char *binary_alloc(size_t size)
{
    char *ptr = malloc(size + 1);
    ptr[size] = 0;
    malloc_stacks[malloc_idx] = ptr;
    malloc_idx++;
    malloc_stacks[malloc_idx] = NULL;
    return ptr;
}

void binary_free()
{
    while (malloc_idx--)
    {
        free(malloc_stacks[malloc_idx]);
    }
}
