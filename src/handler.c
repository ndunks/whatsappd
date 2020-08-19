#include "handler.h"

CONTACT_NAME *names = NULL;
static WebMessageInfo msg;

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

// handle and add message as unread
static CHAT *handle_message_unread(BINARY_NODE *node, bool is_latest)
{
    CHAT *chat = NULL;
    proto_parse_WebMessageInfo(&msg, node->child.data, node->child_len);
    if (wid_is_user(msg.key->remoteJid) && !msg.key->fromMe)
    {
        chat = chats_add_unread(msg.key->remoteJid, &msg);
        if (is_latest && msg.key->id != NULL)
            strcpy(chat->last_msg_id, msg.key->id);
    }
    proto_free_WebMessageInfo(&msg);

    return chat;
}

static int handle_action_add_before(BINARY_NODE **nodes, int node_count)
{
    int i;
    BINARY_NODE *child;
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
                chats_add_msg(chat, &msg);
            }
        }

        proto_free_WebMessageInfo(&msg);
    }
    return 0;
}

static int handle_action_add_last(BINARY_NODE **nodes, int node_count)
{
    //BINARY_NODE *child;
    for (int i = 0; i < node_count; i++)
    {
        handle_message_unread(nodes[i], true);
        // child = nodes[i];
        // proto_parse_WebMessageInfo(&msg, child->child.data, child->child_len);
        // if (wid_is_user(msg.key->remoteJid) && !msg.key->fromMe)
        //     chats_add_unread(msg.key->remoteJid, &msg);
        // proto_free_WebMessageInfo(&msg);
    }

    return 0;
}

static int handle_action(BINARY_NODE *node)
{
    BINARY_NODE *child;
    BINARY_ACTION_ADD add = 0;
    HANDLE *handle;
    CHAT *chat;
    int i;
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

    if (strcmp("message", child->tag) != 0)
    {
        handle = handler_get(child->tag);
        if (handle == NULL)
            warn("handle_action: don't know to handle tag %s", child->tag);
        else
        {
            for (i = 0; i < node->child_len; i++)
                (*handle->function)(node->child.list[i]);
        }
        return 0;
    }

    // Only handle message!
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

    case BINARY_ACTION_ADD_RELAY:

        for (i = 0; i < node->child_len; i++)
        {
            chat = handle_message_unread(node->child.list[i], true);

            // ignore empty, its may deleted msg
            if (chat->msg[chat->unread_count] != NULL)
                chat->unread_count++;
        }

        return 0;
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

    // if (strcmp("user", child->tag) == 0)
    // {
    //     warn("handle_response Skip user/contact");
    //     return 0;
    // }
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

static int handle_user(BINARY_NODE *node)
{
    uint64_t jid_num;
    char *val;
    CONTACT_NAME *new;

    if ((val = binary_attr(node, "jid")) == NULL)
        return 0;

    if (!wid_is_user(val))
        return 0;

    jid_num = helper_jid_to_num(val);

    if ((val = binary_attr(node, "name")) == NULL &&
        (val = binary_attr(node, "notify")) == NULL &&
        (val = binary_attr(node, "short")) == NULL &&
        (val = binary_attr(node, "vname")) == NULL)
        return 0;

    new = calloc(sizeof(CONTACT_NAME), 1);
    new->jid_num = jid_num;
    new->name = malloc(strlen(val) + 1);
    strcpy(new->name, val);

    if (names != NULL)
        new->next = names;

    names = new;
    return 0;
}
// Only type = frequent ??
// static int handle_contacts(BINARY_NODE *node)
// {
//     BINARY_NODE *child;
//     int i;
//     info("handle_contacts %s %d %d %d", node->tag, node->attr_len, node->child_len, node->child_type);
//     binary_print_attr(node);
//     for (i = 0; i < node->child_len; i++)
//     {
//         child = node->child.list[i];
//         info("    %s %d %d %d", child->tag, child->attr_len, child->child_len, child->child_type);
//         binary_print_attr(child);
//     }
//     return 0;
// }

static int handle_chat(BINARY_NODE *node)
{
    char *jid, *unread, *count_str;
    CHAT *chat;
    int unread_count = 0;

    jid = binary_attr(node, "jid");
    unread = binary_attr(node, "unread");
    count_str = binary_attr(node, "count");

    if (!wid_is_user(jid))
        return 0;

    //binary_print_attr(node);
    if (count_str)
    {
        unread_count = atoi(count_str);
        if (unread_count > 0)
        {
            chat = chats_add_unread(jid, NULL);
            chat->unread_count = unread_count;
            accent("%s: %u unread!", jid, unread_count);
        }
    }

    // Skip group chat
    if (!wid_is_user(jid))
        return 0;

    if (unread != NULL && *unread == '1')
    {
        ok("GOTTT UNREAD CHAT!");
        chats_add_unread(jid, NULL);
    }
    return 0;
}

// handle and add message as unread
static int handle_message(BINARY_NODE *node)
{
    return handle_message_unread(node, false) == NULL;
}

HANDLE handler_tag[] = {
    {.tag = "action", .function = handle_action},
    {.tag = "response", .function = handle_response},
    {.tag = "chat", .function = handle_chat},
    {.tag = "user", .function = handle_user},
    {.tag = "message", .function = handle_message},
    //{.tag = "contacts", .function = handle_contacts},
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

    info("handler_preempt_read\n-------------------");
    while (1)
    {

        ret = wss_ssl_check_read(500);
        if (ret <= 0)
            break;

        CHECK(wasocket_read(&data, &tag, &size));

        if (wss_frame_rx.opcode != WS_OPCODE_BINARY)
        {
            if (size < 64)
            {
                info("%s", data);
            }
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
    info("handler_preempt_read: %d\n-------------------", preempt_count);
    handler_preempt_post();

    return preempt_count == 0;
}

void handler_preempt_post()
{
    CHAT *chat;
    CONTACT_NAME *name;
    // fill name from names
    chat = chats;
    while (chat != NULL)
    {
        if (chat->name == NULL)
        {
            name = names;
            while (name != NULL)
            {
                if (name->jid_num == chat->jid_num)
                {
                    chat->name = name->name;
                    name->name = NULL; // prevent from freeing
                    break;
                }
                name = name->next;
            }
        }
        chat = chat->next;
    };

    //clearing names
    while (names != NULL)
    {
        name = names;
        names = name->next;
        if (name->name != NULL)
            free(name->name);
        free(name);
    }
}