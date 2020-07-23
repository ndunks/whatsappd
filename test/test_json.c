#include <json.h>
#include "test.h"

#define STR(str) #str
int test_json_field()
{
    char *ptr,
        json_str[] = " {    \"fie\\\"ld1\":1234,\"field2\"  : [1,2,3,4], \n"
                     "\"field3\":true,\"field4\": { \"ch\":[\"123\",\"\"] } ,\t"
                     "\"field5\":123456789 }";
    int i = 0;

    ptr = json_str;
    ZERO(json_parse_object(&ptr));
    TRUTHY(json_len == 5);

    TRUTHY(json[i].key != NULL);
    ZERO(strcmp(json[i].key, "fie\"ld1"));
    TRUTHY(json[i].value != NULL);
    ZERO(strcmp(json[i].value, "1234"));
    info("%s\t-> %s", json[i].key, json[i].value);
    i++;

    TRUTHY(json[i].key != NULL);
    ZERO(strcmp(json[i].key, "field2"));
    TRUTHY(json[i].value != NULL);
    ZERO(strcmp(json[i].value, "[1,2,3,4]"));
    info("%s\t-> %s", json[i].key, json[i].value);
    i++;

    TRUTHY(json[i].key != NULL);
    ZERO(strcmp(json[i].key, "field3"));
    TRUTHY(json[i].value != NULL);
    ZERO(strcmp(json[i].value, "true"));
    info("%s\t-> %s", json[i].key, json[i].value);
    i++;

    TRUTHY(json[i].key != NULL);
    ZERO(strcmp(json[i].key, "field4"));
    TRUTHY(json[i].value != NULL);
    ZERO(strcmp(json[i].value, "{ \"ch\":[\"123\",\"\"] }"));
    info("%s\t-> %s", json[i].key, json[i].value);
    i++;

    TRUTHY(json[i].key != NULL);
    ZERO(strcmp(json[i].key, "field5"));
    TRUTHY(json[i].value != NULL);
    ZERO(strcmp(json[i].value, "123456789"));
    info("%s\t-> %s", json[i].key, json[i].value);
    i++;

    TRUTHY(json_find("field5") == 4);
    TRUTHY(json_has("field5"));
    ZERO(strcmp(json_get("field5"), "123456789"));
    ok("All fields OK\n------------------");

    return 0;
}

int test_json_parse_init()
{
    char *ptr, str[] = "{\n\"status\":200,"
                       "\"ref\":\"6\\u0111Ct\\\"S+A=\\\"=\","
                       "\"ttl\":20000,"
                       "\"update\":false,"
                       "\"curr\":\"2.2029\\\\.4\","
                       "\"time\":1595261640005.0}";
    ptr = str;

    ZERO(json_parse_object(&ptr));
    TRUTHY(json_len == 6);
    accent("test_json_parse_init: Checking fields");

    ZERO(strcmp(json_get("status"), "200"));
    ZERO(strcmp(json_get("ref"), "6\\u0111Ct\"S+A=\"="));
    ZERO(strcmp(json_get("ttl"), "20000"));
    ZERO(strcmp(json_get("update"), "false"));
    ZERO(strcmp(json_get("curr"), "2.2029\\.4"));
    ZERO(strcmp(json_get("time"), "1595261640005.0"));
    ok("test_json_parse_init OK\n------------------");
    return 0;
}
int test_json_parse_conn()
{
    char *ptr, str[] = "[\"Conn\",{\n"
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
    ptr = str;
    ZERO(json_parse_object(&ptr));
    ZERO(strcmp(json_get("ref"), "1@k.."));
    ZERO(strcmp(json_get("wid"), "6285726501017@c.us"));
    ZERO(strcmp(json_get("connected"), "true"));
    ZERO(strcmp(json_get("isResponse"), "false"));
    ZERO(strcmp(json_get("serverToken"), "1@+.."));
    ZERO(strcmp(json_get("browserToken"), "1@3.."));
    ZERO(strcmp(json_get("clientToken"), "dZu.."));
    ZERO(strcmp(json_get("lc"), "ID"));
    ZERO(strcmp(json_get("lg"), "en"));
    ZERO(strcmp(json_get("locales"), "en-ID,id-ID"));
    ZERO(strcmp(json_get("is24h"), "true"));
    ZERO(strcmp(json_get("secret"), "s4kb.."));
    ZERO(strcmp(json_get("protoVersion"), "[0,17]"));
    ZERO(strcmp(json_get("binVersion"), "10"));
    ZERO(strcmp(json_get("battery"), "64"));
    ZERO(strcmp(json_get("plugged"), "false"));
    ZERO(strcmp(json_get("platform"), "iphone"));
    ZERO(strcmp(json_get("features"), "{\"KEY_PARTICIPANT\":true,\"FLAGS\":\"EAE...\"}"));
    return 0;
}

int test_main()
{

    return test_json_field() || test_json_parse_init() || test_json_parse_conn();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
