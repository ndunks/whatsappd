#include "buf.h"

uint8_t *buf = NULL;
size_t buf_idx = 0, buf_len = 0;

void buf_set(char *src, size_t src_len)
{
	buf_idx = 0;
	buf = (uint8_t *)src;
	buf_len = src_len;
}

bool buf_available()
{
	return buf_idx < buf_len;
}

void buf_read_bytes(char *dst, size_t len)
{
	memcpy(dst, buf + buf_idx, len);
	buf_idx += len;
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

uint32_t buf_read_var_int32(unsigned len, const uint8_t *data)
{
	uint32_t num = data[0] & 0x7f;
	if (len > 1)
	{
		num |= ((uint32_t)(data[1] & 0x7f) << 7);
		if (len > 2)
		{
			num |= ((uint32_t)(data[2] & 0x7f) << 14);
			if (len > 3)
			{
				num |= ((uint32_t)(data[3] & 0x7f) << 21);
				if (len > 4)
					num |= ((uint32_t)(data[4]) << 28);
			}
		}
	}
	return num;
}