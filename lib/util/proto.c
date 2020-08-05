#include "proto.h"

uint len_varint(uint64_t num)
{
	uint len = 1;
	while (num >= 0b10000000)
	{
		len++;
		num = num >> 7;
	}
	return len;
}

uint write_varint(uint32_t num)
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

uint32_t read_varint()
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

size_t proto_size(PROTO *proto, int proto_len)
{
	size_t size = 0;
	int i = 0;
	while (i++ < proto_len)
	{
		if (!proto->field)
		{
			proto++;
			continue;
		}

		size += len_tag(proto->field);
		switch (proto->type)
		{
		case WIRETYPE_VARINT:
			size += len_varint(proto->value.num64);
			break;

		case WIRETYPE_FIXED64:
			size += 8;
			break;

		case WIRETYPE_LENGTH_DELIMITED:
			size += len_varint(proto->len) + proto->len;
			break;

		case WIRETYPE_FIXED32:
			size += 4;
			break;

		default:
			warn("Unsupported wiretype: %d", proto->type);
			break;
		}

		proto++;
	}

	return size;
}

int proto_write(PROTO *proto, int proto_len)
{
	int i = 0;
	while (i++ < proto_len)
	{
		if (!proto->field)
		{
			proto++;
			continue;
		}
		if (!buf_available())
		{
			err("proto_write: not enough buffer");
			return BUF_ERR_NOT_ENOUGH;
		}

		write_tag(proto);

		switch (proto->type)
		{
			// 	int32, int64, uint32, uint64, sint32, sint64, bool, enum
		case WIRETYPE_VARINT:
			write_varint(proto->value.num64);
			break;

			// fixed64, sfixed64, double
		case WIRETYPE_FIXED64:
			*(uint64_t *)&buf[buf_idx] = proto->value.num64;
			buf_idx += 8;
			break;

			// string, bytes, embedded messages, packed repeated fields
		case WIRETYPE_LENGTH_DELIMITED:
			write_varint(proto->len);
			memcpy(&buf[buf_idx], proto->value.buf, proto->len);
			buf_idx += proto->len;
			break;

			//	fixed32, sfixed32, float
		case WIRETYPE_FIXED32:
			*(uint32_t *)&buf[buf_idx] = proto->value.num32;
			buf_idx += 4;
			break;

		default:
			warn("Unsupported wiretype: %d", proto->type);
			break;
		}
		proto++;
	}
	return 0;
}
int proto_scan(PROTO *proto, int proto_len, int max_field)
{
	int i = 0;
	//accent("proto_scan %lu fields", max_field);
	while (buf_available())
	{
		if (i == proto_len)
		{
			err("proto_scan: not enough array storage");
			return BUF_ERR_NOT_ENOUGH;
		}

		read_tag(proto);
		if (proto->field == 0)
			break;

		switch (proto->type)
		{
			// 	int32, int64, uint32, uint64, sint32, sint64, bool, enum
		case WIRETYPE_VARINT:
			proto->value.num64 = read_varint();
			break;

			// fixed64, sfixed64, double
		case WIRETYPE_FIXED64:
			proto->value.num64 = *(uint64_t *)&buf[buf_idx];
			buf_idx += 8;
			break;

			// string, bytes, embedded messages, packed repeated fields
		case WIRETYPE_LENGTH_DELIMITED:
			proto->len = read_varint();
			proto->value.buf = (char *)&buf[buf_idx];
			buf_idx += proto->len;
			break;

			//	fixed32, sfixed32, float
		case WIRETYPE_FIXED32:
			proto->value.num32 = *(uint32_t *)&buf[buf_idx];
			buf_idx += 4;
			break;

		default:
			warn("Unsupported wiretype: %d", proto->type);
			break;
		}

		i++;
		if (max_field && max_field == proto->field)
			break;
		proto++;
	}

	return i;
}

PROTO *protos_get(uint32_t field, PROTO *proto, size_t max)
{
	size_t i = 0;
	while (proto->field && i++ < max)
	{
		//accent("get: %lu == %lu", proto->field, field);
		if (proto->field == field)
			return proto;
		proto++;
	}

	return NULL;
}

// Just enough to read text message
int proto_parse_Message(Message *dst, char *buf, size_t buf_size)
{
	PROTO *ptr, scan[1];
	size_t len = 0;

	memset(scan, 0, sizeof(PROTO) * 1);

	buf_set(buf, buf_size);
	len = proto_scan(scan, 1, 1);

	if (len < 0)
		return 1;

	if ((ptr = protos_get(1, scan, 1)) != NULL)
	{
		dst->conversation = binary_alloc(ptr->len);
		strncpy(dst->conversation, ptr->value.buf, ptr->len);
	}
	return 0;
}

int proto_parse_MessageKey(MessageKey *dst, char *buf, size_t buf_size)
{
	PROTO *ptr, scan[4];
	size_t len = 0;

	memset(scan, 0, sizeof(PROTO) * 4);

	buf_set(buf, buf_size);
	len = proto_scan(scan, 4, 4);

	if (len < 0)
		return 1;

	if ((ptr = protos_get(1, scan, 4)) != NULL)
	{
		strncpy(dst->remoteJid, ptr->value.buf, ptr->len);
		dst->remoteJid[ptr->len] = 0;
	}
	if ((ptr = protos_get(2, scan, 4)) != NULL)
	{
		dst->fromMe = ptr->value.num64 & 0b1;
	}
	if ((ptr = protos_get(3, scan, 4)) != NULL)
	{
		strncpy(dst->id, ptr->value.buf, ptr->len);
		dst->id[ptr->len] = 0;
	}
	if ((ptr = protos_get(4, scan, 4)) != NULL)
	{
		strncpy(dst->participant, ptr->value.buf, ptr->len);
		dst->participant[ptr->len] = 0;
	}

	return 0;
}

int proto_write_MessageKey(MessageKey *src)
{
	PROTO *ptr, protos[4];
	size_t len = 0;
	memset(protos, 0, sizeof(protos));

	ptr = &protos[len++];
	ptr->field = 1;
	ptr->type = WIRETYPE_LENGTH_DELIMITED;
	ptr->len = strlen(src->remoteJid);
	ptr->value.buf = src->remoteJid;

	ptr = &protos[len++];
	ptr->field = 2;
	ptr->type = WIRETYPE_VARINT;
	ptr->value.num64 = src->fromMe;

	ptr = &protos[len++];
	ptr->field = 3;
	ptr->type = WIRETYPE_LENGTH_DELIMITED;
	ptr->len = strlen(src->id);
	ptr->value.buf = src->id;

	ptr = &protos[len++];
	ptr->field = 4;
	ptr->type = WIRETYPE_LENGTH_DELIMITED;
	ptr->len = strlen(src->participant);
	ptr->value.buf = src->participant;

	return proto_write(protos, len);
}

int proto_parse_WebMessageInfo(WebMessageInfo *dst, char *buf, size_t buf_size)
{
	PROTO *ptr, scan[19];
	size_t len = 0;

	memset(scan, 0, sizeof(PROTO) * 19);

	buf_set(buf, buf_size);
	len = proto_scan(scan, 19, 0);

	if (len < 0)
		return 1;

	if ((ptr = protos_get(1, scan, 19)) != NULL)

		proto_parse_MessageKey(&dst->key, ptr->value.buf, ptr->len);

	if ((ptr = protos_get(2, scan, 19)) != NULL)
		proto_parse_Message(&dst->message, ptr->value.buf, ptr->len);

	if ((ptr = protos_get(3, scan, 19)) != NULL)
		dst->messageTimestamp = ptr->value.num64;

	if ((ptr = protos_get(4, scan, 19)) != NULL)
		dst->status = ptr->value.num64;

	// err("parseMssage: %d", len);
	// for (int i = 4; i < len; i++)
	// {
	// 	info("msg field: %d, type %d", scan[i].field, scan[i].type);
	// };

	return 0;
}
int proto_write_WebMessageInfo(WebMessageInfo *src)
{
	PROTO *ptr, protos[4];
	size_t len = 0;
	memset(protos, 0, sizeof(protos));
}