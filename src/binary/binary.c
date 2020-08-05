#include "helper.h"
#include "binary.h"
const char wa_host_short[5] = "c.us",
           wa_host_long[15] = "s.whatsapp.net";

const char *const DICTIONARY_SINGLEBYTE[] = {
    0, 0, 0, "200", "400", "404", "500", "501", "502", "action", "add", "after", "archive", "author", "available", "battery", "before", "body", "broadcast", "chat", "clear", "code", "composing", "contacts", "count", "create", "debug", "delete", "demote", "duplicate", "encoding", "error", "false", "filehash", "from", "g.us", "group", "groups_v2", "height", "id", "image", "in",
    "index", "invis", "item", "jid", "kind", "last", "leave", "live", "log", "media", "message", "mimetype", "missing", "modify", "name", "notification", "notify", "out", "owner", "participant", "paused", "picture", "played", "presence", "preview", "promote", "query", "raw", "read", "receipt", "received", "recipient", "recording", "relay", "remove", "response", "resume", "retry", "s.whatsapp.net", "seconds",
    "set", "size", "status", "subject", "subscribe", "t", "text", "to", "true", "type", "unarchive", "unavailable", "url", "user", "value", "web", "width", "mute", "read_only", "admin", "creator", "short", "update", "powersave", "checksum", "epoch", "block", "previous", "409", "replaced", "reason", "spam", "modify_tag",
    "message_info", "delivery", "emoji", "title", "description", "canonical-url", "matched-text", "star", "unstar", "media_key", "filename", "identity", "unread", "page", "page_count", "search", "media_message", "security", "call_log", "profile",

    // singleByteAppends
    // Dictionary: a("cbhhieidga")
    "ciphertext", "invite",
    // Dictionary: a("dabidjgjaj")
    "gif", "vcard", "frequent",
    // Dictionary: a("bbijaajigb")
    "privacy", "blacklist", "whitelist", "verify",
    // Dictionary: a("ceihfbbggi")
    "location", "document", "elapsed", "revoke_invite", "expiration", "unsubscribe", "disable",
    // Dictionary: a("ddfgdjiacb")
    "vname", "old_jid", "new_jid", "announcement", "locked", "prop", "label", "color", "call", "offer", "call-id",
    /*
        {
            Writer: a("bfidcjhfda"),
            Reader: a("cdjdachjig"),
            Dictionary: a("gjcabeieb")
        }
        */
    "quick_reply", "sticker", "pay_t", "accept", "reject", "sticker_pack", "invalid", "canceled", "missed", "connected", "result", "audio", "video", "recent"};

int DICTIONARY_SINGLEBYTE_LEN = sizeof(DICTIONARY_SINGLEBYTE) / sizeof(void *);

static int malloc_idx = 0;
static void *malloc_stacks[BINARY_MALLOC_MAX] = {0};

void *binary_alloc(size_t size)
{
    if (malloc_idx == BINARY_MALLOC_MAX)
    {
        die("BINARY_MALLOC_MAX");
    }

    char *ptr = malloc(size + 1);
    ptr[size] = 0;
    malloc_stacks[malloc_idx] = ptr;
    malloc_idx++;
    malloc_stacks[malloc_idx] = NULL;
    return (void *)ptr;
}

void binary_alloc_stat()
{
    accent("BINARY allocated %d times.", malloc_idx);
}

void binary_free()
{
    while (malloc_idx--)
    {
        free(malloc_stacks[malloc_idx]);
    }
}

void binary_print_attr(BINARY_NODE *node)
{
    accent("print_attr: %s", node->tag);
    for (int i = 0; i < node->attr_len; i++)
    {
        printf("%s: %s\n", node->attrs[i].key, node->attrs[i].value);
    }
    accent("--------------");
}

char *binary_attr(BINARY_NODE *node, const char *key)
{
    for (int i = 0; i < node->attr_len; i++)
    {
        if (strcmp(key, node->attrs[i].key) == 0)
            return node->attrs[i].value;
    }
    return NULL;
}

BINARY_NODE *binary_child(BINARY_NODE *node, int index)
{
    if (node->child_type != BINARY_NODE_CHILD_LIST)
    {
        warn("Not list child undefined: %d", node->child_type);
        return NULL;
    }

    if (index == node->child_len)
    {
        warn("Index undefined: %d", index);
        return NULL;
    }
    return *(node->child.list + index);
}

BINARY_ACTION_ADD binary_get_action_add(const char *add)
{
    if (add == NULL)
        return BINARY_ACTION_ADD_NONE;

    switch (add[0])
    {
    case 'r':
        return BINARY_ACTION_ADD_RELAY;
    case 'l':
        return BINARY_ACTION_ADD_LAST;
    case 'b':
        return BINARY_ACTION_ADD_BEFORE;
    case 'a':
        return BINARY_ACTION_ADD_AFTER;
    case 'u':
        return add[1] == 'p' ? BINARY_ACTION_ADD_UPDATE : BINARY_ACTION_ADD_UNREAD;
    default:
        return BINARY_ACTION_ADD_NONE;
    }
}