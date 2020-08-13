#include "wa.h"

int wa_send_text(const char *number, const char *const text)
{
    MessageKey key;
    Message msg;
    WebMessageInfo info;
    size_t buf_len, node_size, text_len = strlen(text), sent_size;
    char *bin_buf, *node_buf, tag[80] = {0};
    BINARY_NODE *node, child;
    buf_len = text_len + 512;

    memset(&child, 0, sizeof(BINARY_NODE));

    bin_buf = malloc(buf_len);
    node_buf = malloc(buf_len);

    key.fromMe = 1;
    wa_create_msg_id(key.id);
    wa_sanitize_jid_long(key.remoteJid, number);

    msg.conversation = (char *)text;

    info.key = &key;
    info.message = &msg;
    info.status = WEB_MESSAGE_INFO_STATUS_PENDING;
    info.messageTimestamp = time(NULL);

    buf_set(bin_buf, buf_len);
    proto_write_WebMessageInfo(&info);

    child.tag = "message";
    child.child_type = BINARY_NODE_CHILD_BINARY;
    child.child_len = buf_idx;
    child.child.data = bin_buf;

    node = binary_node_action("relay", &child);
    node_size = binary_write(node, node_buf, buf_len);

    sprintf(tag, "%s,", key.id);
    sent_size = wasocket_send_binary(node_buf, node_size, tag, BINARY_METRIC_MESSAGE, 0);

    free(bin_buf);
    free(node_buf);
    binary_node_action_free(node);

    CHECK(sent_size == 0);
    return wa_reply_json_ok(tag);
}