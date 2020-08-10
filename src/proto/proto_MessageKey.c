#include "proto.h"

int proto_parse_MessageKey(MessageKey *dst, char *buf, size_t buf_size)
{
	PROTO *ptr, scan[4];
	size_t len = 0;

	memset(scan, 0, sizeof(PROTO) * 4);

	buf_set(buf, buf_size);
	len = proto_scan(scan, 4, 4);

	if (len <= 0)
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

int proto_write_MessageKey(MessageKey *src, PROTO *protos, int proto_len)
{
	PROTO *ptr;
	int len = 0;
	if (proto_len < 4)
	{
		err("proto_write_MessageKey: not enough proto array");
		return BUF_ERR_NOT_ENOUGH;
	}

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

	if (src->participant[0] != 0)
	{
		ptr = &protos[len++];
		ptr->field = 4;
		ptr->type = WIRETYPE_LENGTH_DELIMITED;
		ptr->len = strlen(src->participant);
		ptr->value.buf = src->participant;
	}
	return len;
}

void proto_free_MessageKey(MessageKey *src)
{
}
