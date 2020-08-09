#include "proto.h"

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
	{
		dst->key = calloc(sizeof(MessageKey), 1);
		proto_parse_MessageKey(dst->key, ptr->value.buf, ptr->len);
	}

	if ((ptr = protos_get(2, scan, 19)) != NULL)
	{
		dst->message = calloc(sizeof(Message), 1);
		proto_parse_Message(dst->message, ptr->value.buf, ptr->len);
	}

	if ((ptr = protos_get(3, scan, 19)) != NULL)
		dst->messageTimestamp = ptr->value.num32;

	if ((ptr = protos_get(4, scan, 19)) != NULL)
		dst->status = ptr->value.num64;

	// err("parseMssage: %d", len);
	// for (int i = 4; i < len; i++)
	// {
	// 	info("msg field: %d, type %d", scan[i].field, scan[i].type);
	// };
	return 0;
}

// You must set buf_set before call this
int proto_write_WebMessageInfo(WebMessageInfo *src)
{
	PROTO proto, childs[4];
	int childs_len;
	size_t size;

	if (src->key != NULL)
	{
		memset(childs, 0, sizeof(childs));
		childs_len = proto_write_MessageKey(src->key, childs, 4);
		if (childs_len < 0)
			return childs_len;
		proto.field = 1;
		proto.type = WIRETYPE_LENGTH_DELIMITED;
		size = proto_size(childs, childs_len);
		proto_tag_write(&proto);
		proto_varint_write(size);
		proto_writes(childs, childs_len);
	}

	if (src->message != NULL)
	{
		memset(childs, 0, sizeof(childs));
		childs_len = proto_write_Message(src->message, childs, 4);
		if (childs_len < 0)
			return childs_len;
		proto.field = 2;
		proto.type = WIRETYPE_LENGTH_DELIMITED;
		size = proto_size(childs, childs_len);
		proto_tag_write(&proto);
		proto_varint_write(size);
		proto_writes(childs, childs_len);
	}

	if (src->messageTimestamp > 0)
	{
		proto.field = 3;
		proto.type = WIRETYPE_VARINT;
		proto.value.num64 = src->messageTimestamp;
		info("TS %lu = %u", proto.value.num64, src->messageTimestamp);
		proto_write(&proto);
		hexdump(&buf[buf_idx - 9], 9);
	}

	if (src->status > WEB_MESSAGE_INFO_STATUS_ERROR &&
		src->status <= WEB_MESSAGE_INFO_STATUS_PLAYED)
	{
		proto.field = 4;
		proto.type = WIRETYPE_VARINT;
		proto.value.num32 = src->status;
		proto_write(&proto);
	}

	return 0;
}
void proto_free_WebMessageInfo(WebMessageInfo *src)
{
	if (src->key != NULL)
	{
		proto_free_MessageKey(src->key);
		free(src->key);
	}

	if (src->message != NULL)
	{
		proto_free_Message(src->message);
		free(src->message);
	}
}