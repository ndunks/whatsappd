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
    TRUTHY(proto_varint_write(num) == 1);
    buf_idx = 0;
    TRUTHY(proto_varint_read() == num);
    buf_idx = 0;

    num = 0x7f;
    TRUTHY(proto_varint_write(num) == 1);
    buf_idx = 0;
    TRUTHY(proto_varint_read() == num);
    buf_idx = 0;

    num = 0x80;
    TRUTHY(proto_varint_write(num) == 2);
    buf_idx = 0;
    TRUTHY(proto_varint_read() == num);
    buf_idx = 0;

    num = 0xfaba;
    TRUTHY(proto_varint_write(num) == 3);
    buf_idx = 0;
    TRUTHY(proto_varint_read() == num);
    buf_idx = 0;

    num = 0xffff80;
    TRUTHY(proto_varint_write(num) == 4);
    buf_idx = 0;
    TRUTHY(proto_varint_read() == num);
    buf_idx = 0;

    num = 0xfaba0000u;
    TRUTHY(proto_varint_write(num) == 5);
    buf_idx = 0;
    TRUTHY(proto_varint_read() == num);
    buf_idx = 0;

    num = 0xffffffffu;
    TRUTHY(proto_varint_write(num) == 5);
    buf_idx = 0;
    TRUTHY(proto_varint_read() == num);
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

    num = proto_tag_write(&proto);
    TRUTHY(num == 1);
    TRUTHY(buf[0] == 0b00001101);

    buf_idx = 0;
    proto.field = 0xffffff;
    proto_write(&proto);
    proto_write(&proto);
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
    num = proto_tag_write(&proto);
    TRUTHY(num == 2);
    TRUTHY((uint8_t)buf[0] == 0b11111010u);
    buf_idx = 0;
    proto_tag_read(&scan[0]);
    ptr = &scan[0];
    TRUTHY(proto.field == ptr->field);
    TRUTHY(proto.type == ptr->type);

    buf_idx = 0;
    proto_write(&proto);
    proto_write(&proto);
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

    proto.field = 3;
    proto.type = WIRETYPE_VARINT;
    proto.value.num64 = 1589871077llu;
    buf_idx = 0;
    num = proto_write(&proto);
    TRUTHY(num == 0);
    buf_idx = 0;
    proto_scan(scan, 1, 3);
    ptr = &scan[0];

    TRUTHY(proto.field == ptr->field);
    TRUTHY(proto.type == ptr->type);
    TRUTHY(proto.value.num64 == ptr->value.num64);

    return 0;
}

int test_proto()
{
    WebMessageInfo msg;
    char mybuff[800];

    memset(&msg, 0, sizeof(WebMessageInfo));
    memset(&msg, 0, sizeof(WebMessageInfo));

    ZERO(load_sample("sent-message.proto.bin", read_buf, BUF_SIZE, &read_size));
    ZERO(proto_parse_WebMessageInfo(&msg, read_buf, read_size));
    TRUTHY(msg.status == 1);
    TRUTHY(msg.messageTimestamp == 1589871077);

    FALSY(msg.key == NULL);
    info("remoteJid: %s", msg.key->remoteJid);
    ZERO(strcmp(msg.key->remoteJid, "628997026464@s.whatsapp.net"));
    TRUTHY(msg.key->fromMe == true);
    ZERO(strcmp(msg.key->id, "3EB082A541839959E947"));
    FALSY(msg.message == NULL);
    ZERO(strcmp(msg.message->conversation, "tttest"));

    buf_set(mybuff, 800);
    TRUTHY(proto_write_WebMessageInfo(&msg) >= 0);
    TRUTHY(read_size == buf_idx);
    //info("ReadSize: %lu, WriteSize: %lu", read_size, buf_idx);
    //helper_save_file("tmp/test-message.proto.bin", mybuff, buf_idx);
    //hexdump(mybuff, buf_idx);
    //fwrite(mybuff, 1, buf_idx, stderr);
    //info("------");
    ZERO(memcmp(mybuff, read_buf, buf_idx));

    proto_free_WebMessageInfo(&msg);

    return 0;
}

int test_main()
{
    return test_varint() || test_write() || test_proto();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
