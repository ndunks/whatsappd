#include <proto.h>
#include "test.h"
#define BUF_SIZE 80000

static char read_buf[BUF_SIZE];
static size_t read_size;

int test_varint()
{
    char buf[64];
    uint32_t num;

    buf_set(buf, 64);

    num = 1;
    TRUTHY(write_varint(num) == 1);
    buf_idx = 0;
    TRUTHY(read_varint() == num);
    buf_idx = 0;

    num = 0x7f;
    TRUTHY(write_varint(num) == 1);
    buf_idx = 0;
    TRUTHY(read_varint() == num);
    buf_idx = 0;

    num = 0x80;
    TRUTHY(write_varint(num) == 2);
    buf_idx = 0;
    TRUTHY(read_varint() == num);
    buf_idx = 0;

    num = 0xfaba;
    TRUTHY(write_varint(num) == 3);
    buf_idx = 0;
    TRUTHY(read_varint() == num);
    buf_idx = 0;

    num = 0xffff80;
    TRUTHY(write_varint(num) == 4);
    buf_idx = 0;
    TRUTHY(read_varint() == num);
    buf_idx = 0;

    num = 0xfaba0000u;
    TRUTHY(write_varint(num) == 5);
    buf_idx = 0;
    TRUTHY(read_varint() == num);
    buf_idx = 0;

    num = 0xffffffffu;
    TRUTHY(write_varint(num) == 5);
    buf_idx = 0;
    TRUTHY(read_varint() == num);
    buf_idx = 0;

    return 0;
}

int test_write()
{
    PROTO proto, *ptr, scan[5];
    uint32_t num;
    char buf[64];

    buf_set(buf, 64);

    memset(&proto, 0, sizeof(proto));
    memset(scan, 0, sizeof(scan));

    proto.field = 1;
    proto.type = WIRETYPE_FIXED32;
    proto.value.num32 = 0xfaab;

    num = write_tag(&proto);
    TRUTHY(num == 1);
    TRUTHY(buf[0] == 0b00001101);

    buf_idx = 0;
    proto.field = 0xffffff;
    proto_write(&proto, 1);
    proto_write(&proto, 1);
    buf_idx = 0;
    proto_scan(scan, 5, 5);
    ptr = &scan[0];

    TRUTHY(proto.field == ptr->field);
    TRUTHY(proto.type == ptr->type);
    TRUTHY(proto.value.num32 == ptr->value.num32);
    ptr++;
    TRUTHY(proto.field == ptr->field);
    TRUTHY(proto.type == ptr->type);
    TRUTHY(proto.value.num32 == ptr->value.num32);

    // LENGTH DELIMITED
    proto.field = 0xff;
    proto.type = WIRETYPE_LENGTH_DELIMITED;
    proto.len = 14;
    proto.value.buf = "THIS_MY_BUFFER";
    buf_idx = 0;
    num = write_tag(&proto);
    TRUTHY(num == 2);
    TRUTHY((uint8_t)buf[0] == 0b11111010u);
    buf_idx = 0;
    read_tag(&scan[0]);
    ptr = &scan[0];
    TRUTHY(proto.field == ptr->field);
    TRUTHY(proto.type == ptr->type);

    buf_idx = 0;
    proto_write(&proto, 1);
    proto_write(&proto, 1);
    hexdump(buf, 16);

    buf_idx = 0;
    proto_scan(scan, 5, 5);
    ptr = &scan[0];
    info("BUF: %s", ptr->value.buf);
    TRUTHY(proto.field == ptr->field);
    TRUTHY(proto.len == ptr->len);
    ZERO(strncmp(proto.value.buf, ptr->value.buf, proto.len));
    ptr++;
    info("BUF: %s", ptr->value.buf);
    TRUTHY(proto.field == ptr->field);
    TRUTHY(proto.len == ptr->len);
    ZERO(strncmp(proto.value.buf, ptr->value.buf, proto.len));

    return 0;
}

int test_proto()
{
    WebMessageInfo msg;
    memset(&msg, 0, sizeof(WebMessageInfo));

    ZERO(load_sample("sent-message.proto.bin", read_buf, BUF_SIZE, &read_size));
    ZERO(proto_parse_WebMessageInfo(&msg, read_buf, read_size));
    TRUTHY(msg.status == 1);
    TRUTHY(msg.messageTimestamp == 1589871077);

    FALSY(*(char *)&msg.key == 0);
    info("remoteJid: %s", msg.key.remoteJid);
    ZERO(strcmp(msg.key.remoteJid, "628997026464@s.whatsapp.net"));
    TRUTHY(msg.key.fromMe == true);
    ZERO(strcmp(msg.key.id, "3EB082A541839959E947"));
    FALSY(*(char *)&msg.message == 0);
    ZERO(strcmp(msg.message.conversation, "tttest"));

    return 0;
}

int test_main()
{
    // return test_varint() || test_write() || test_proto();
    return test_proto();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
