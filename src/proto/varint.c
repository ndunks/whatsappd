#include "proto.h"

uint proto_varint_size(uint64_t num)
{
	uint len = 1;
	while (num >= 0b10000000)
	{
		len++;
		num = num >> 7;
	}
	return len;
}

uint proto_varint_write(uint32_t num)
{
	uint32_t start = buf_idx;

	while (num >= 0b10000000)
	{
		buf[buf_idx++] = num | 0b10000000;
		num = num >> 7;
	}
	buf[buf_idx++] = num;
	return buf_idx - start;
}

uint32_t proto_varint_read()
{
	uint32_t val = 0lu;
	int i = 0, shift = 0;
	uint8_t *ptr = &buf[buf_idx];

	do
	{
		val |= ((ptr[i] & 0b01111111UL) << shift);
		shift += 7;
		if (i >= 5)
		{
			err("read_var_length overflow");
			break;
		}
	} while (ptr[i++] & 0b10000000);

	buf_idx += i;
	return val;
}
