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