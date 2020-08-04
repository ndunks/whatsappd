#include <proto.h>
#include "test.h"
#define BUF_SIZE 80000

static char read_buf[BUF_SIZE];
static size_t read_size;

int test_main()
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
    // Where id come from?
    ZERO(strcmp(msg.key.id, "3EB082A541839959E947"));

    FALSY(*(char *)&msg.message == 0);
    // Content of the message
    ZERO(strcmp(msg.message.conversation, "tttest"));
    return 0;
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
