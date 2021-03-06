#include "proto.h"

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

		size += proto_tag_size(proto->field);
		switch (proto->type)
		{
		case WIRETYPE_VARINT:
			size += proto_varint_size(proto->value.num64);
			break;

		case WIRETYPE_FIXED64:
			size += 8;
			break;

		case WIRETYPE_LENGTH_DELIMITED:
			size += proto_varint_size(proto->len) + proto->len;
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

int proto_write(PROTO *proto)
{
	if (!proto->field)
	{
		warn("proto_write: no field number");
		return 0;
	}

	if (!buf_available())
	{
		err("proto_write: not enough buffer");
		return BUF_ERR_NOT_ENOUGH;
	}

	proto_tag_write(proto);

	switch (proto->type)
	{
		// 	int32, int64, uint32, uint64, sint32, sint64, bool, enum
	case WIRETYPE_VARINT:
		proto_varint_write(proto->value.num64);
		break;

		// fixed64, sfixed64, double
	case WIRETYPE_FIXED64:
		*(uint64_t *)&buf[buf_idx] = proto->value.num64;
		buf_idx += 8;
		break;

		// string, bytes, embedded messages, packed repeated fields
	case WIRETYPE_LENGTH_DELIMITED:
		proto_varint_write(proto->len);
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
	return 0;
}

int proto_writes(PROTO *proto, int proto_len)
{
	int i = 0, ret;
	while (i++ < proto_len)
	{
		if ((ret = proto_write(proto++)) < 0)
			return ret;
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

		proto_tag_read(proto);
		if (proto->field == 0)
			break;

		switch (proto->type)
		{
			// 	int32, int64, uint32, uint64, sint32, sint64, bool, enum
		case WIRETYPE_VARINT:
			proto->value.num64 = proto_varint_read();
			break;

			// fixed64, sfixed64, double
		case WIRETYPE_FIXED64:
			proto->value.num64 = *(uint64_t *)&buf[buf_idx];
			buf_idx += 8;
			break;

			// string, bytes, embedded messages, packed repeated fields
		case WIRETYPE_LENGTH_DELIMITED:
			proto->len = proto_varint_read();
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
