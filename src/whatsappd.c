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

int whatsappd_presence(int available)
{
    BINARY_NODE node, *childs, child;
    BINARY_NODE_ATTR *attr;
    size_t node_size, sent_size;
    char *node_buf = malloc(256), *reply;

    memset(&node, 0, sizeof(BINARY_NODE));
    memset(&child, 0, sizeof(BINARY_NODE));

    node.tag = "action";
    attr = &node.attrs[node.attr_len++];
    attr->key = "type";
    attr->value = "set";
    attr = &node.attrs[node.attr_len++];
    attr->key = "epoch";
    attr->value = helper_epoch();

    child.tag = "presence";
    attr = &child.attrs[child.attr_len++];
    attr->key = "type";
    attr->value = available ? "available" : "unavailable";

    node.child_len = 1;
    node.child_type = BINARY_NODE_CHILD_LIST;
    node.child.list = &childs;
    childs = &child;

    node_size = binary_write(&node, node_buf, 256);

    sent_size = wasocket_send_binary(node_buf, node_size, NULL, BINARY_METRIC_PRESENCE, EPHEMERAL_IGNORE | EPHEMERAL_AVAILABLE);
    free(node_buf);

    CHECK(sent_size == 0);
    reply = wasocket_read_reply(NULL);
    CHECK(json_parse_object(&reply));
    return strncmp(json_get("status"), "200", 3);
}

int whatsappd_send_text(const char *number, const char *const text)
{
    MessageKey key;
    Message msg;
    WebMessageInfo info;
    size_t buf_len, node_size, text_len = strlen(text), sent_size;
    char *bin_buf, *node_buf, tag[16] = {0};
    BINARY_NODE node_action, node_message;
    BINARY_NODE_ATTR *attr;
    buf_len = text_len + 512;

    memset(&node_action, 0, sizeof(BINARY_NODE));
    memset(&node_message, 0, sizeof(BINARY_NODE));
    //node_message = childs;

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

    node_action.tag = "action";

    attr = &node_action.attrs[node_action.attr_len++];
    attr->key = "type";
    attr->value = "relay";

    attr = &node_action.attrs[node_action.attr_len++];
    attr->key = "epoch";
    attr->value = helper_epoch();

    node_message.tag = "message";
    node_message.child_type = BINARY_NODE_CHILD_BINARY;
    node_message.child_len = buf_idx;
    node_message.child.data = bin_buf;

    node_action.child_type = BINARY_NODE_CHILD_LIST;
    node_action.child_len = 1;
    node_action.child.list = malloc(sizeof(void *));
    node_action.child.list[0] = &node_message;

    node_size = binary_write(&node_action, node_buf, buf_len);

    sprintf(tag, "%s,", key.id);
    sent_size = wasocket_send_binary(node_buf, node_size, tag, BINARY_METRIC_MESSAGE, 0);
    TRY(sent_size == 0);
    wasocket_read_all(500);

CATCH:
    free(node_action.child.list);
    free(bin_buf);
    free(node_buf);
    return CATCH_RET;
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
    TRY(whatsappd_presence(1));
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