#include <helper.h>
#include "test.h"

#define STR(str) #str
int test_json_field()
{
    char *field = NULL, *value = NULL, *ptr,
         json[] = " {    \"fie\\\"ld1\":1234,\"field2\"  : [1,2,3,4], \n"
                  "\"field3\":true,\"field4\": { \"ch\":[\"123\",\"\"] } ,\t"
                  "\"field5\":123456789 }";

    ptr = json;

    field = helper_json_field(&ptr);
    //accent("field: %s, next: %s\n---", field, ptr);
    TRUTHY(field != NULL);
    ZERO(strcmp(field, "fie\"ld1"));
    TRUTHY(*ptr == ':');

    value = helper_json_value(&ptr);
    //accent("value: %s, next: %s\n---", value, ptr);
    TRUTHY(value != NULL);
    ZERO(strcmp(value, "1234"));
    TRUTHY(*ptr == '"');

    field = helper_json_field(&ptr);
    //accent("field: %s, next: %s\n---", field, ptr);
    TRUTHY(field != NULL);
    ZERO(strcmp(field, "field2"));

    value = helper_json_value(&ptr);
    //accent("value: %s, next: %s\n---", value, ptr);
    TRUTHY(value != NULL);
    ZERO(strcmp(value, "[1,2,3,4]"));

    field = helper_json_field(&ptr);
    //accent("field: %s, next: %s\n---", field, ptr);
    TRUTHY(field != NULL);
    ZERO(strcmp(field, "field3"));

    value = helper_json_value(&ptr);
    //accent("value: %s, next: %s\n---", value, ptr);
    TRUTHY(value != NULL);
    ZERO(strcmp(value, "true"));

    field = helper_json_field(&ptr);
    //accent("field: %s, next: %s\n---", field, ptr);
    TRUTHY(field != NULL);
    ZERO(strcmp(field, "field4"));

    value = helper_json_value(&ptr);
    //accent("value: %s, next: %s\n---", value, ptr);
    TRUTHY(value != NULL);
    ZERO(strcmp(value, "{ \"ch\":[\"123\",\"\"] }"));

    field = helper_json_field(&ptr);
    //accent("field: %s, next: %s\n---", field, ptr);
    TRUTHY(field != NULL);
    ZERO(strcmp(field, "field5"));

    value = helper_json_value(&ptr);
    //accent("value: %s, next: %s\n---", value, ptr);
    TRUTHY(value != NULL);
    ZERO(strcmp(value, "123456789"));

    return 0;
}

int test_json_parse_init()
{
    struct HELPER_JSON_INIT_REPLY json;
    char str[] = "{\"status\":200,"
                 "\"ref\":\"6\\u0111Ct\\\"S+A=\\\"=\","
                 "\"ttl\":20000,"
                 "\"update\":false,"
                 "\"curr\":\"2.2029\\\\.4\","
                 "\"time\":1595261640005.0}";

    memset(&json, 0, sizeof(json));

    ZERO(helper_parse_init_reply(&json, str));

    ZERO(strcmp(json.status, "200"));
    ZERO(strcmp(json.ref, "6\\u0111Ct\"S+A=\"="));
    ZERO(strcmp(json.ttl, "20000"));
    ZERO(strcmp(json.update, "false"));
    ZERO(strcmp(json.curr, "2.2029\\.4"));
    ZERO(strcmp(json.time, "1595261640005.0"));

    return 0;
}
int test_json_parse_conn()
{
    struct HELPER_JSON_INIT_CONN json;
    char str[] = "[\"Conn\",{"
                 "\"ref\":\"1@k..\","
                 "\"wid\":\"6285726501017@c.us\","
                 "\"connected\":true,"
                 "\"isResponse\":\"false\","
                 "\"serverToken\":\"1@+..\","
                 "\"browserToken\":\"1@3..\","
                 "\"clientToken\":\"dZu..\","
                 "\"lc\":\"ID\","
                 "\"lg\":\"en\","
                 "\"locales\":\"en-ID,id-ID\"," // <-- watch this have comma
                 "\"is24h\":true,"
                 "\"secret\":\"s4kb..\","
                 "\"protoVersion\":[0,17]," // <-- watch this have comma
                 "\"binVersion\":10,"
                 "\"battery\":64,"
                 "\"plugged\":false,"
                 "\"platform\":\"iphone\","
                 "\"features\":{\"KEY_PARTICIPANT\":true,\"FLAGS\":\"EAE...\"}}]";

    memset(&json, 0, sizeof(json));

    ZERO(helper_parse_conn(&json, str));
    ZERO(strcmp(json.ref, "1@k.."));
    ZERO(strcmp(json.wid, "6285726501017@c.us"));
    ZERO(strcmp(json.connected, "true"));
    ZERO(strcmp(json.isResponse, "false"));
    ZERO(strcmp(json.serverToken, "1@+.."));
    ZERO(strcmp(json.browserToken, "1@3.."));
    ZERO(strcmp(json.clientToken, "dZu.."));
    ZERO(strcmp(json.lc, "ID"));
    ZERO(strcmp(json.lg, "en"));
    ZERO(strcmp(json.locales, "en-ID,id-ID"));
    ZERO(strcmp(json.is24h, "true"));
    ZERO(strcmp(json.secret, "s4kb.."));
    ZERO(strcmp(json.protoVersion, "[0,17]"));
    ZERO(strcmp(json.binVersion, "10"));
    ZERO(strcmp(json.battery, "64"));
    ZERO(strcmp(json.plugged, "false"));
    ZERO(strcmp(json.platform, "iphone"));
    ZERO(strcmp(json.features, "{\"KEY_PARTICIPANT\":true,\"FLAGS\":\"EAE...\"}"));
    return 0;
}

int test_qrcode()
{
    const char test[] = "1@K0X+UNPXH94guVvB2S+oOOszeMUtaOyr+zn8LTEevbJf3qRsEmHy5/Fvi+JECJS9zR0xiLHCp0wRvA==,"
                        "77jrZhfCD51b1PLGsw0MbBWQDYVFZI0pmJz9pvq81ng=,"
                        "hOxZa3unwWFZ5eY3RTK+Eg==";

    ZERO(helper_qrcode_show(test));
    return 0;
}

int test_main()
{

    return test_json_field() || test_json_parse_init() || test_json_parse_conn()
        //|| test_qrcode()
        ;
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
