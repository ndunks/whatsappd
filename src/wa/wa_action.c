#include "wa.h"

int wa_action_presence(int available)
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

int wa_action_read(const char *jid, const char *index, uint count)
{
    BINARY_NODE *node, child;
    size_t node_size, sent_size;
    char jid_sanitized[64] = {0}, count_str[12] = {0}, node_buf[1024] = {0};

    if (index == NULL)
    {
        warn("wa_action_read: No index!");
        return 0;
    }

    memset(&child, 0, sizeof(BINARY_NODE));
    sprintf(count_str, "%u", count);
    wa_sanitize_jid_long(jid_sanitized, jid);

    child.tag = "read";
    child.attrs[0].key = "jid";
    child.attrs[0].value = jid_sanitized;
    child.attrs[1].key = "index";
    child.attrs[1].value = (char *)index;
    child.attrs[2].key = "count";
    child.attrs[2].value = count_str;
    child.attrs[3].key = "owner";
    child.attrs[3].value = "false";
    child.attr_len = 4;

    node = binary_node_action("set", &child);
    node_size = binary_write(node, node_buf, 1024);

    sent_size = wasocket_send_binary(node_buf, node_size, NULL, BINARY_METRIC_READ, EPHEMERAL_IGNORE | EPHEMERAL_AVAILABLE);

    binary_node_action_free(node);

    CHECK(sent_size == 0);
    return wa_reply_json_ok(NULL);
}

int wa_action_presence_check(const char *number, WA_PRESENCE_CHECK_RESULT *result)
{
    char *val, jid[64], jid_reply[64], buf[256];
    size_t len, sent_size;

    wa_sanitize_jid_short(jid, number);

    len = sprintf(buf, ",[\"action\",\"presence\",\"subscribe\",\"%s\"]", jid);

    sent_size = wasocket_send_text(buf, len, wasocket_short_tag());

    CHECK(sent_size == 0);

    if (wa_reply_json_ok(NULL) != 0)
        return 1;

    ////s13,["Presence",{"id":"6285726501018@c.us","type":"unavailable","t":1597328601,"deny":false}]

    len = wasocket_read_command_reply("Presence", buf, 256);
    if (len == 0)
        return 1;
    val = buf;
    CHECK(json_parse_object(&val));

    if ((val = json_get("id")) == NULL)
    {
        warn("wa_presence_check: json no id");
        return 1;
    }

    wa_sanitize_jid_short(jid_reply, val);

    if (strcmp(jid, jid_reply) != 0)
    {
        warn("wa_presence_check: %s != %s", jid, jid_reply);
        return 1;
    }

    if ((val = json_get("type")) != NULL)
    {
        strcpy(result->type, val);
    }

    result->time = (long)json_get_number("t");
    result->deny = json_get_bool("deny");

    //draining other replies
    wasocket_read_all(500);
    return 0;
}