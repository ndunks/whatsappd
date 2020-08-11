#include "handler.h"

static bool wid_is_user(char *jid)
{
    char *host = strchr(jid, '@');
    if (host++)
    {
        if (strncmp(host, wa_host_short, 5) == 0)
            return true;
        if (strncmp(host, wa_host_long, 15) == 0)
            return true;
    }

    return false;
}

static int handle_action_add_before(BINARY_NODE **nodes, int node_count)
{
    int i;
    BINARY_NODE *child;
    WebMessageInfo msg;
    CHAT *chat;

    for (i = node_count - 1; i >= 0; i--)
    {
        child = nodes[i];

        proto_parse_WebMessageInfo(&msg, child->child.data, child->child_len);

        if (wid_is_user(msg.key->remoteJid) && !msg.key->fromMe)
        {
            chat = chats_get(msg.key->remoteJid);
        
            if (chat != NULL && chat->unread_count > 0 &&
                chat->msg_count < chat->unread_count)
            {
                chats_add_msg(chat, msg.message);
            }
        }

        proto_free_WebMessageInfo(&msg);
    }
    return 0;
}

static int handle_action_add_last(BINARY_NODE **nodes, int node_count)
{
    int i;
    BINARY_NODE *child;
    WebMessageInfo msg;

    for (i = 0; i < node_count; i++)
    {
        child = nodes[i];

        proto_parse_WebMessageInfo(&msg, child->child.data, child->child_len);

        if (wid_is_user(msg.key->remoteJid) && !msg.key->fromMe)
            chats_add_unread(msg.key->remoteJid, NULL, msg.message);

        proto_free_WebMessageInfo(&msg);
    }
    return 0;
}

static int handle_action(BINARY_NODE *node)
{
    BINARY_NODE *child;
    BINARY_ACTION_ADD add = 0;
    char *add_str;

    if (node->child_type != BINARY_NODE_CHILD_LIST)
    {
        warn("handle_action not list type");
        return 1;
    }

    if (!node->child_len)
    {
        warn("handle_action no child");
        return 1;
    }

    child = binary_child(node, 0);

    if (strcmp("message", child->tag))
    {
        warn("handle_action: don't know to handle tag %s", child->tag);
        return 0;
    }

    add_str = binary_attr(node, "add");
    add = binary_get_action_add(add_str);

    info("handle_action add: %s", add_str);
    switch (add)
    {
    case BINARY_ACTION_ADD_BEFORE:
        return handle_action_add_before(node->child.list, node->child_len);
    // AFAIK, is unread message that not been synced to this WA Web
    // so it sent just once!
    case BINARY_ACTION_ADD_LAST:
        return handle_action_add_last(node->child.list, node->child_len);
    default:
        warn("handle_action: Ignored %s", add_str);
        break;
    }

    return 0;
}

static int handle_response(BINARY_NODE *node)
{
    BINARY_NODE *child;
    HANDLE *handle;
    int i;
    if (node->child_type != BINARY_NODE_CHILD_LIST)
    {
        warn("handle_response not list type");
        return 1;
    }

    if (!node->child_len)
    {
        warn("handle_response no child");
        return 1;
    }
    child = binary_child(node, 0);

    if (strcmp("user", child->tag) == 0)
    {
        warn("handle_response Skip user/contact");
        return 0;
    }
    handle = handler_get(child->tag);

    if (handle == NULL)
    {
        warn("handle_response unhandled: %s", child->tag);
        return 1;
    }

    for (i = 0; i < node->child_len; i++)
    {
        (*handle->function)(node->child.list[i]);
    }

    return 0;
}

static int handle_chat(BINARY_NODE *node)
{
    char *jid, *name, *unread, *count_str;
    CHAT *chat;
    int unread_count = 0;

    unread = binary_attr(node, "unread");
    jid = binary_attr(node, "jid");
    name = binary_attr(node, "name");
    count_str = binary_attr(node, "count");
    //binary_print_attr(node);
    if (count_str)
    {
        unread_count = atoi(count_str);
        if (unread_count > 0)
        {
            chat = chats_add_unread(jid, name, NULL);
            chat->unread_count = unread_count;
            accent("%s: %u unread!", jid, unread_count);
        }
    }

    if (!wid_is_user(jid))
    {
        //info("Skip group chat");
        return 0;
    }

    if (unread != NULL && *unread == '1')
    {
        ok("GOTTT UNREAD CHAT!");
        chats_add_unread(jid, name, NULL);
    }
    return 0;
}

HANDLE handler_tag[] = {
    {.tag = "action", .function = handle_action},
    {.tag = "response", .function = handle_response},
    {.tag = "chat", .function = handle_chat},
    {NULL}};

HANDLE *handler_get(const char *tag)
{
    HANDLE *ptr;
    int tag_len = strlen(tag);

    ptr = handler_tag;
    do
    {
        if (strncmp(tag, ptr->tag, tag_len))
            continue;
        return ptr;
    } while (((++ptr)->tag) != NULL);
    return NULL;
}

int handler_handle(BINARY_NODE *node)
{
    const HANDLE *handle = handler_get(node->tag);

    if (handle == NULL)
    {
        warn("Unhandled: %s", node->tag);
        return 1;
    }

    info("* handler_%s()", node->tag);
    return (*handle->function)(node);
}

int handler_preempt()
{
    BINARY_NODE *node;
    char *data, *tag;
    ssize_t size;
    int preempt_count = 0;
    int ret;

    while (1)
    {

        ret = wss_ssl_check_read(500);
        if (ret <= 0)
            break;

        //info("handler_preempt_read\n-------------------");
        CHECK(wasocket_read(&data, &tag, &size));

        if (wss_frame_rx.opcode != WS_OPCODE_BINARY)
        {
            info("%s", data);
            warn("handler_preempt ignored non binary");
            continue;
        }

        node = binary_read(data, size);
        if (node == NULL)
            return 1;

        handler_handle(node);

        if (strncmp("preempt-", tag, 8) == 0)
        {
            preempt_count++;
        }
        binary_free();
    }

    return preempt_count != 2;
}
