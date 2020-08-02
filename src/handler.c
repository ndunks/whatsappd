#include <helper.h>
#include <binary.h>
#include <proto.h>

#include "handler.h"

size_t handler_unread_count = 0;
CHAT *handler_unread_chats = NULL;

static bool wid_is_user(char *jid)
{
    char *host = strchr(jid, '@');
    if (host++)
    {
        if (strncmp(host, wa_host_long, 15) == 0)
            return true;
        if (strncmp(host, wa_host_short, 5) == 0)
            return true;
    }

    return false;
}

static bool wid_is_group(char *jid)
{
    char *host = strchr(jid, '@');
    if (host++)
    {
        if (strcmp(host, "g.us") == 0)
            return true;
    }
    return false;
}

static int handle_action(BINARY_NODE *node)
{
    char *add = binary_attr(node, "add");
    // switch (attr.add) {
    //         // Realtime new message?
    //         case "relay":
    //         case "update":
    //             msg = this.parseWebMessageInfo(childs[0], "relay") as WebMessageInfo.AsObject
    //             if (!msg) {
    //                 L(Color.r("<< Action IGNORED"), attr.add, attr, childs)
    //                 return
    //             }
    //             if (!msg.key.fromme) {
    //                 const jid = widHelper.parse(msg.key.remotejid)
    //                 switch (jid.server) {
    //                     case 'c.us':
    //                         this.emit('new-user-message', msg)
    //                         break;

    //                     case 'g.us':
    //                         this.emit('new-group-message', msg)
    //                         break;
    //                     case 'broadcast':
    //                         if (jid.user == 'status') {
    //                             this.emit('status-broadcast', msg)
    //                             break;
    //                         }
    //                     default:
    //                         L(Color.y('Unknown sender message'), jid, msg.message)
    //                         break;
    //                 }
    //             } else {
    //                 L(attr.add, msg.key, msg.message)
    //             }
    //             store.getChat(msg.key.remotejid).addMessage({
    //                 key: msg.key,
    //                 direction: msg.key.fromme ? 'out' : 'in',
    //                 message: msg.message,
    //             })
    //             break

    // if ((attr = binary_attr(node, "add")) != NULL)
    // {
    //     accent("handle_action: %s", attr);
    // }
    // else
    // {
    //     accent("handle_action: NULL");
    // }
    //binary_print_attr(node);
    for (int i = 0; i < node->child_len; i++)
    {
        handler_handle(node->child.list[i]);
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
    //accent("handle_response: %s %u %ld", handle->tag, node->child_len, node->child_type);
    for (i = 0; i < node->child_len; i++)
    {
        (*handle->function)(node->child.list[i]);
    }

    return 0;
}

static int handle_message(BINARY_NODE *node)
{
    WebMessageInfo msg;
    // filter only unread received message
    if (node->child_type != BINARY_NODE_CHILD_BINARY)
    {
        warn("handle_message not binary type");
        return 0;
    }
    memset(&msg, 0, sizeof(WebMessageInfo));
    proto_parse_WebMessageInfo(&msg, node->child.data, node->child_len);
    if (!wid_is_user(msg.key.remoteJid))
    {
        //warn("Ignore non-user msg: %s", msg.key.remoteJid);
        return 0;
    }
    if (msg.key.fromMe)
    {
        //warn("Ignore self message");
        return 0;
    }

    //accent("MSG %s: " COL_YELL "%s " COL_GREEN "%s", msg.key.id, msg.key.remoteJid, msg.key.fromMe ? "ME" : "");
    //accent("MSG %s:" COL_YELL "\n%s " COL_GREEN "%s\n" COL_NORM "%s\n---------", msg.key.id, msg.key.remoteJid, msg.key.fromMe ? "ME" : "", msg.message.conversation);

    return 0;
}

static int handle_user(BINARY_NODE *node)
{
    accent("handle_user: %s", node->tag);
    return 0;
}

static int handle_chat(BINARY_NODE *node)
{
    char *jid, *name, *unread;

    unread = binary_attr(node, "unread");
    jid = binary_attr(node, "jid");
    name = binary_attr(node, "name");

    if (!wid_is_user(jid))
    {
        //info("Skip group chat");
        return 0;
    }

    //binary_print_attr(node);
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
    {.tag = "message", .function = handle_message},
    {.tag = "user", .function = handle_user},
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
        if (tail)
        {
            tail->next = chat;
        }
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

    return (*handle->function)(node);
}
