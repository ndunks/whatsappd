#include "binary.h"

char *DICTIONARY_SINGLEBYTE[] = {
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
