#include "wa.h"

char *wa_query_profile_pic_thumb(const char *number)
{
    char jid[64], buf[256];
    size_t len, sent_size;

    wa_sanitize_jid_short(jid, number);
    len = sprintf(buf, ",[\"query\",\"ProfilePicThumb\",\"%s\"]", jid);

    sent_size = wasocket_send_text(buf, len, wasocket_short_tag());
    if (sent_size == 0)
        return NULL;

    // todo: get the url
    wasocket_read_all(500);
    return NULL;
}

bool wa_query_exist(const char *number)
{
    char jid[64], buf[256];
    size_t len, sent_size;

    wa_sanitize_jid_short(jid, number);
    len = sprintf(buf, "[\"query\",\"exist\",\"+%s\"]", jid);

    sent_size = wasocket_send_text(buf, len, wasocket_short_tag());
    CHECK(sent_size == 0);
    return wa_reply_json_ok(NULL) == 0;
}
