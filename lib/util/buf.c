#include "buf.h"

uint8_t *buf = NULL;
size_t buf_idx = 0, buf_len = 0;

void buf_set(char *src, size_t src_len)
{
    buf_idx = 0;
    buf = (uint8_t *)src;
    buf_len = src_len;
}

uint8_t buf_read_byte()
{
    return buf[buf_idx++];
}

uint16_t buf_read_int16()
{
    buf_idx += 2;
    return be16toh(*(uint16_t *)&buf[buf_idx - 2]);
}

uint32_t buf_read_int20()
{
    uint32_t value = (buf[buf_idx++] & 0xf) << 16;
    value |= (buf[buf_idx++] & 0xff) << 8;
    value |= (buf[buf_idx++] & 0xff);
    return value;
}

uint32_t buf_read_int32()
{
    buf_idx += 4;
    return be32toh(*(uint32_t *)&buf[buf_idx - 4]);
}
