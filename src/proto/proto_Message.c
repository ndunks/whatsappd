#include "proto.h"


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
