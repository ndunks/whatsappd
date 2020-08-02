#include "helper.h"
#include "proto.h"

// size_t get_tag_size(uint32_t number)
// {
// 	if (number < (1UL << 4))
// 		return 1;
// 	else if (number < (1UL << 11))
// 		return 2;
// 	else if (number < (1UL << 18))
// 		return 3;
// 	else if (number < (1UL << 25))
// 		return 4;
// 	else
// 		return 5;
// }

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

size_t read_varint()
{
	size_t val = 0;
	int i = 0, shift = 0;
	uint8_t *ptr = &buf[buf_idx];

	do
	{
		val |= ((ptr[i] & 0b01111111) << shift);
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

int proto_scan(PROTO *proto, size_t proto_len, size_t max_field)
{
	int i = 0;
	//accent("proto_scan %lu fields", max_field);
	while (buf_available())
	{
		if (i == proto_len)
		{
			err("proto_scan: not enough arr");
			return BUF_ERR_NOT_ENOUGH;
		}

		read_tag(proto);
		//info("tag: %d, field: %d", proto->type, proto->field);

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
			proto->value.buf = &buf[buf_idx];
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
		strncpy(dst->remoteJid, ptr->value.buf, ptr->len);
	if ((ptr = protos_get(2, scan, 4)) != NULL)
		dst->fromMe = ptr->value.num64 & 0b1;
	if ((ptr = protos_get(3, scan, 4)) != NULL)
		strncpy(dst->id, ptr->value.buf, ptr->len);
	if ((ptr = protos_get(4, scan, 4)) != NULL)
		strncpy(dst->participant, ptr->value.buf, ptr->len);

	return 0;
}

int proto_parse_WebMessageInfo(WebMessageInfo *dst, char *buf, size_t buf_size)
{
	PROTO *ptr, scan[15];
	size_t len = 0;

	memset(scan, 0, sizeof(PROTO) * 15);

	buf_set(buf, buf_size);
	len = proto_scan(scan, 15, 4);

	if (len < 0)
		return 1;

	if ((ptr = protos_get(1, scan, 15)) != NULL)

		proto_parse_MessageKey(&dst->key, ptr->value.buf, ptr->len);

	if ((ptr = protos_get(2, scan, 15)) != NULL)
		proto_parse_Message(&dst->message, ptr->value.buf, ptr->len);

	if ((ptr = protos_get(3, scan, 15)) != NULL)
		dst->messageTimestamp = ptr->value.num64;

	if ((ptr = protos_get(4, scan, 15)) != NULL)
		dst->status = ptr->value.num64;

	return 0;
}