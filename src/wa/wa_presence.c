#include "wa.h"

int wa_presence(int available)
{
    BINARY_NODE *node, child;
    size_t node_size, sent_size;
    char *node_buf = malloc(256);

    memset(&child, 0, sizeof(BINARY_NODE));

    child.tag = "presence";
    child.attrs[0].key = "type";
    child.attrs[0].value = available ? "available" : "unavailable";
    child.attr_len = 1;

    node = binary_node_action("set", &child);
    node_size = binary_write(node, node_buf, 256);

    sent_size = wasocket_send_binary(node_buf, node_size, NULL, BINARY_METRIC_PRESENCE, EPHEMERAL_IGNORE | EPHEMERAL_AVAILABLE);

    free(node_buf);
    binary_node_action_free(node);

    CHECK(sent_size == 0);
    return wa_reply_json_ok(NULL);
}

int wa_presence_check(const char *number)
{
    char *jid[64], buf[256];
    size_t node_size, msg_len, sent_size;
    wa_sanitize_jid_short(jid, number);
    msg_len = sprintf(buf, ",[\"action\",\"presence\",\"subscribe\",\"%s\"]", jid);

    sent_size = wasocket_send_text(buf, msg_len, wasocket_short_tag());

    CHECK(sent_size == 0);
    if (wa_reply_json_ok(NULL) != 0)
    {
        return 1;
    }

    return 0;
}