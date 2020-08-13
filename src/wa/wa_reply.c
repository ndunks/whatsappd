#include "wa.h"

int wa_reply_json_ok(char *req_tag)
{
    char *reply;
    reply = wasocket_read_reply(req_tag);
    CHECK(json_parse_object(&reply));
    return strncmp(json_get("status"), "200", 3);
}
