#include <time.h>
#include "whatsappd.h"

// 20 bytes msg id + 1 for NULL. not support image id 16 bytes
void whatsappd_create_msg_id(const char *str)
{
    char randBytes[10];
    memset((void *)str, 0, 21);
    crypto_random(randBytes, 10);
    helper_buf_to_hex((uint8_t *)str, (uint8_t *)randBytes, 10);
}

void whatsappd_sanitize_jid(char *dst, const char *number)
{
    char *at;
    strcpy(dst, number);
    at = strchr(dst, '@');

    if (at == NULL)
        strcat(dst, "@");
    else
    {
        if (strncmp(at + 1, wa_host_long, 14) == 0)
            return;

        *(at + 1) = 0;
    }

    strcat(dst, wa_host_long);
}

int whatsappd_reply_json_ok(char *req_tag)
{
    char *reply;
    reply = wasocket_read_reply(req_tag);
    CHECK(json_parse_object(&reply));
    return strncmp(json_get("status"), "200", 3);
}

int whatsappd_presence(int available)
{
    BINARY_NODE node, *childs, child;
    size_t node_size, sent_size;
    char *node_buf = malloc(256);

    memset(&node, 0, sizeof(BINARY_NODE));
    memset(&child, 0, sizeof(BINARY_NODE));

    node.tag = "action";
    node.attrs[0].key = "type";
    node.attrs[0].value = "set";
    node.attrs[1].key = "epoch";
    node.attrs[1].value = helper_epoch();
    node.attr_len = 2;

    child.tag = "presence";
    child.attrs[0].key = "type";
    child.attrs[0].value = available ? "available" : "unavailable";
    child.attr_len = 1;

    node.child_len = 1;
    node.child_type = BINARY_NODE_CHILD_LIST;
    node.child.list = &childs;
    childs = &child;

    node_size = binary_write(&node, node_buf, 256);
    info("node: %lu", node_size);
    hexdump(node_buf, node_size);

    sent_size = wasocket_send_binary(node_buf, node_size, NULL, BINARY_METRIC_PRESENCE, EPHEMERAL_IGNORE | EPHEMERAL_AVAILABLE);
    free(node_buf);

    CHECK(sent_size == 0);
    return whatsappd_reply_json_ok(NULL);
}

int whatsappd_send_text(const char *number, const char *const text)
{
    MessageKey key;
    Message msg;
    WebMessageInfo info;
    size_t buf_len, node_size, text_len = strlen(text), sent_size;
    char *bin_buf, *node_buf, tag[80] = {0};
    BINARY_NODE node, child, *childs;
    buf_len = text_len + 512;

    memset(&node, 0, sizeof(BINARY_NODE));
    memset(&child, 0, sizeof(BINARY_NODE));

    bin_buf = malloc(buf_len);
    node_buf = malloc(buf_len);

    key.fromMe = 1;
    whatsappd_create_msg_id(key.id);
    whatsappd_sanitize_jid(key.remoteJid, number);

    msg.conversation = (char *)text;

    info.key = &key;
    info.message = &msg;
    info.status = WEB_MESSAGE_INFO_STATUS_PENDING;
    info.messageTimestamp = time(NULL);

    buf_set(bin_buf, buf_len);
    proto_write_WebMessageInfo(&info);

    node.tag = "action";

    node.attrs[0].key = "type";
    node.attrs[0].value = "relay";
    node.attrs[1].key = "epoch";
    node.attrs[1].value = helper_epoch();
    node.attr_len = 2;

    child.tag = "message";
    child.child_type = BINARY_NODE_CHILD_BINARY;
    child.child_len = buf_idx;
    child.child.data = bin_buf;

    node.child_type = BINARY_NODE_CHILD_LIST;
    node.child_len = 1;
    node.child.list = &childs;
    childs = &child;

    node_size = binary_write(&node, node_buf, buf_len);

    sprintf(tag, "%s,", key.id);
    sent_size = wasocket_send_binary(node_buf, node_size, tag, BINARY_METRIC_MESSAGE, 0);

    free(bin_buf);
    free(node_buf);
    CHECK(sent_size == 0);
    return whatsappd_reply_json_ok(tag);
}

/**
 * Login or resume session
 */
int whatsappd_init(const char *config_path)
{
    int cfg_status;
    CFG cfg;

    memset(&cfg, 0, sizeof(CFG));

    cfg_status = cfg_file(config_path);
    if (cfg_status < 0)
    {
        err("Cannot Read/Write %s", cfg_file_get());
        return 1;
    }

    if (cfg_status == 1)
        TRY(cfg_load(&cfg));

    TRY(crypto_init());
    TRY(session_init(&cfg));
    cfg_save(&cfg);
    TRY(handler_preempt());

#ifdef DEBUG
    TRY(whatsappd_presence(1));
#else
    if (whatsappd_presence(1) != 0)
        err("Send presence fail.");
#endif

    CATCH_RET = 0;

CATCH:
    return CATCH_RET;
}

void whatsappd_free()
{
    session_free();
    crypto_free();
}

#ifndef TEST

int main(int argc, char const *argv[])
{
    // todo: arg parser
    TRY(whatsappd_init(NULL))

    //TRY(wss_connect());

CATCH:
    whatsappd_free();
    return CATCH_RET;
}

#endif