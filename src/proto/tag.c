#include "proto.h"

size_t len_tag(uint32_t field)
{
	if (field < (1UL << 4))
		return 1;
	else if (field < (1UL << 11))
		return 2;
	else if (field < (1UL << 18))
		return 3;
	else if (field < (1UL << 25))
		return 4;
	else
		return 5;
}

int read_tag(PROTO *proto)
{
	uint shift = 4;

	proto->type = buf[buf_idx] & 0b00000111;
	proto->field = (buf[buf_idx] & 0b01111111) >> 3;

	while (buf[buf_idx++] & 0b10000000)
	{
		proto->field |= (buf[buf_idx] & 0b01111111) << shift;
		shift += 7;
	}
	return 0;
}

int write_tag(PROTO *proto)
{
	uint32_t field = proto->field;
	size_t start = buf_idx;

	buf[buf_idx] = proto->type & 0b00000111;
	buf[buf_idx++] |= (field & 0b1111) << 3;

	if (field > 0b1111)
	{
		buf[buf_idx - 1] |= 0b10000000;
		field = field >> 4;
		while (field & 0b10000000)
		{
			buf[buf_idx++] = field;
			field = field >> 7;
		}
		buf[buf_idx++] = field;
	}

	return buf_idx - start;
}
