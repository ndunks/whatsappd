#include "proto.h"

// Just enough to read text message
int proto_parse_Message(Message *dst, char *buf, size_t buf_size)
{
	PROTO *ptr, scan[4];
	size_t len = 0;

	memset(scan, 0, sizeof(PROTO) * 4);
	memset(dst, 0, sizeof(Message));

	buf_set(buf, buf_size);
	len = proto_scan(scan, 4, 0);

	if (len <= 0)
		return 1;

	if ((ptr = protos_get(1, scan, 4)) != NULL)
	{
		dst->conversation = malloc(ptr->len + 1);
		strncpy(dst->conversation, ptr->value.buf, ptr->len);
		dst->conversation[ptr->len] = 0;
	}
	return 0;
}

int proto_write_Message(Message *src, PROTO *protos, int proto_len)
{
	PROTO *ptr;
	int len = 0;
	if (src->conversation != NULL)
	{
		ptr = &protos[len++];
		ptr->field = 1;
		ptr->type = WIRETYPE_LENGTH_DELIMITED;
		ptr->len = strlen(src->conversation);
		ptr->value.buf = src->conversation;
	}

	return len;
}

void proto_free_Message(Message *src)
{
	if (src->conversation != NULL)
	{
		free(src->conversation);
	}
}

size_t proto_size_Message(Message *src)
{
	size_t len = 0;
	if (src->conversation != NULL)
	{
		len = proto_tag_size(1);
		len += strlen(src->conversation);
	}
	return len;
}