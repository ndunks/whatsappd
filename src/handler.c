#include "handler.h"

// Unread chats count
size_t handler_unread_count = 0;
// Get FIFO unread chats
CHAT *handler_unread_chats = NULL;

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

static int handle_action(BINARY_NODE *node)
{
    BINARY_NODE *child;
    //HANDLE *handle;
    int msg_len;
    BINARY_ACTION_ADD add = 0;
    char *add_str;
    WebMessageInfo msg;

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

    //handle = handler_get(child->tag);
    add_str = binary_attr(node, "add");
    add = binary_get_action_add(add_str);

    info("handle_action add: %s", add_str);
    switch (add)
    {
    //case BINARY_ACTION_ADD_BEFORE:
    case BINARY_ACTION_ADD_LAST:
        for (int i = 0; i < node->child_len; i++)
        {
            child = node->child.list[i];
            proto_parse_WebMessageInfo(&msg, child->child.data, child->child_len);
            if (!wid_is_user(msg.key.remoteJid))
            {
                warn("Ignore non-user msg: %s", msg.key.remoteJid);
                continue;
            }
            if (msg.key.fromMe)
            {
                warn("Ignore self message");
                continue;
            }
            msg_len = strlen(msg.message.conversation);
            if (msg_len)
                handler_add_unread(msg.key.remoteJid, NULL, msg.message.conversation, msg_len);
            else
                warn("Ignore zero-length last message");
        }
        break;
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

    //accent("handle_response: %s %d %d", handle->tag, node->child_len, node->child_type);

    for (i = 0; i < node->child_len; i++)
    {
        (*handle->function)(node->child.list[i]);
    }

    return 0;
}

static int handle_chat(BINARY_NODE *node)
{
    char *jid, *name, *unread, *count_str;
    size_t unread_count = 0;

    unread = binary_attr(node, "unread");
    jid = binary_attr(node, "jid");
    name = binary_attr(node, "name");
    count_str = binary_attr(node, "count");
    //binary_print_attr(node);
    if (count_str)
    {
        unread_count = atol(count_str);
        if (unread_count)
        {
            handler_add_unread(jid, name, NULL, 0);
            accent("%s: %lu unread!", jid, unread_count);
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
        handler_add_unread(jid, name, NULL, 0);
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

void handler_add_unread(const char *jid, const char *name, char *msg, size_t msg_len)
{
    uint64_t jid_num;
    int i;
    CHAT *tail = NULL, *chat = handler_unread_chats;
    jid_num = helper_jid_to_num(jid);

    while (chat != NULL)
    {
        if (chat->jid_num == jid_num)
            break;
        tail = chat;
        chat = chat->next;
    }

    if (chat == NULL)
    {
        chat = calloc(sizeof(CHAT), 1);
        strcpy(chat->jid, jid);
        chat->jid_num = jid_num;

        if (tail != NULL)
            tail->next = chat;
        else
            handler_unread_chats = chat;

        handler_unread_count++;
    }

    if (name != NULL)
        strcpy(chat->name, name);

    if (msg != NULL)
    {
        if (chat->msg_count == HANDLER_MAX_CHAT_MESSAGE)
        {
            warn("Too many unread msg in chat, msg truncated.");
            free(chat->msg[0]);
            chat->msg_count = HANDLER_MAX_CHAT_MESSAGE - 1;

            i = 0;

            for (i = 0; i < HANDLER_MAX_CHAT_MESSAGE - 2; i++)
            {
                chat->msg[i] = chat->msg[i + 1];
            }
        }

        chat->msg[chat->msg_count] = malloc(msg_len);
        strncpy(chat->msg[chat->msg_count], msg, msg_len);
        chat->msg_count++;
    }
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

        ret = ssl_check_read(500);
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
