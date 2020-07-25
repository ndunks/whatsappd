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
    char *ptr, str[] = "[\"Conn\",{"
                       "\"ref\":\"cPpK0lGO2KXhVIXrehgVRmEgBBQjOCeQYW7bWtLze1ySwQXZ3fq0W1Tr4j844Y93B7FAPFf53J3DVJCTuutwPiNxq5g809+iZaWTAzqHIY++6Y2Cr4aBUjijqoFvBB3bWW2cAGwxOVZ0diHcDhRYB7LaKV/VGfr2sZMdWO+5//PW/t8WUDRYyMQS187xKiYU\","
                       "\"wid\":\"6285726501017@c.us\","
                       "\"connected\":true,"
                       "\"isResponse\":\"false\","
                       "\"serverToken\":\"DBfJ4PCV1kLbQ2up0ZoKFhqKXlHVx/2Zdh6MzfDDVz8FfMIpRmnrodCuAsClz2D2TBktxKLqyWl0rBbJqu7dyQ==\","
                       "\"browserToken\":\"1@qKJVYiKP81i8+i8RQaQZaI7MmtMbfCI7mNBSt+vUaQAKZ3cKAekG0rFkr1tfFru1K0xwKUvluUw6MA==\","
                       "\"clientToken\":\"1@4migVn2az+QmVKdiO7LSSfPyPNteqkBtgIgDNwT6tRHuHsEvRcYKwFD+40LxObIprO7j6eMZI1HupxwGFhNuv/DNELIlGVN73gLtZjZ9k77q4zDaQ6k7LK/0yqqLMNEK0vUFaYsYUYhq3v5UJNUQWw==\","
                       "\"lc\":\"ID\","
                       "\"lg\":\"en\","
                       "\"locales\":\"en-ID,id-ID\","
                       "\"is24h\":true,"
                       "\"secret\":\"s4kb..\","
                       "\"protoVersion\":[0,17],"
                       "\"binVersion\":10,"
                       "\"battery\":64,"
                       "\"plugged\":false,"
                       "\"platform\":\"iphone\","
                       "\"features\":{\"KEY_PARTICIPANT\":true,\"FLAGS\":\"EAE...\"}}]";
    ptr = str;
    ZERO(json_parse_object(&ptr));
    ZERO(strcmp(json_get("ref"), "cPpK0lGO2KXhVIXrehgVRmEgBBQjOCeQYW7bWtLze1ySwQXZ3fq0W1Tr4j844Y93B7FAPFf53J3DVJCTuutwPiNxq5g809+iZaWTAzqHIY++6Y2Cr4aBUjijqoFvBB3bWW2cAGwxOVZ0diHcDhRYB7LaKV/VGfr2sZMdWO+5//PW/t8WUDRYyMQS187xKiYU"));
    ZERO(strcmp(json_get("wid"), "6285726501017@c.us"));
    ZERO(strcmp(json_get("connected"), "true"));
    ZERO(strcmp(json_get("isResponse"), "false"));
    ZERO(strcmp(json_get("serverToken"), "DBfJ4PCV1kLbQ2up0ZoKFhqKXlHVx/2Zdh6MzfDDVz8FfMIpRmnrodCuAsClz2D2TBktxKLqyWl0rBbJqu7dyQ=="));
    ZERO(strcmp(json_get("browserToken"), "1@qKJVYiKP81i8+i8RQaQZaI7MmtMbfCI7mNBSt+vUaQAKZ3cKAekG0rFkr1tfFru1K0xwKUvluUw6MA=="));
    ZERO(strcmp(json_get("clientToken"), "1@4migVn2az+QmVKdiO7LSSfPyPNteqkBtgIgDNwT6tRHuHsEvRcYKwFD+40LxObIprO7j6eMZI1HupxwGFhNuv/DNELIlGVN73gLtZjZ9k77q4zDaQ6k7LK/0yqqLMNEK0vUFaYsYUYhq3v5UJNUQWw=="));
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
    ok("test_json_parse_conn OK\n------------------");
    return 0;
}
int test_json_parse_conn_takeover()
{
    char *ptr, str[] = "[\"Conn\",{"
                       "\"ref\": \"1@N9QMCQb2tmj/JTss5bwEVyDj9Xl1U6uvjlXUnhe0LkmOQ9p761T2CtKvTXAZOcRdEbOVGgiFs37P8Q==\","
                       "\"wid\": \"6285640992980@c.us\","
                       "\"connected\": true,"
                       "\"isResponse\": \"true\","
                       "\"serverToken\": \"1@GGJVQhXxUBmXONlj2WS2C52gmG+6h+yG1urYJl4oRKhbtMJTGicetPymkC5Qj9TQ2xQN8Yt/yedUCg==\","
                       "\"browserToken\": \"1@ufhB49yNHOS+2WpsIjlMDL/TrkAM6T+JW8bdwrD2y2ICQGHRD7W5igWiKBF6kAMBKi8kvELJsfGoeig4YY0tjHCJkdsAqoAkuH2/WyeTVPVXwMY6pHTafWBVOYPv/uOIWqYoUvDvk9yU6cRRoLS80Q==\","
                       "\"clientToken\": \"QNZq7et2EO7ZTBOv6quFvC9F4oCPqE/Q22mZLzAktmU=\","
                       "\"lc\": \"ID\","
                       "\"lg\": \"in\","
                       "\"locales\": \"in-ID\","
                       "\"is24h\": true,"
                       "\"protoVersion\": [0,17],"
                       "\"binVersion\": 11,"
                       "\"battery\": 74,"
                       "\"plugged\": false,"
                       "\"platform\": \"android\","
                       "\"features\": { \"URL\": true,\"FLAGS\": \"EAEYASgBOAFAAUgBWAFgAWgBeAGYAQGgAQGwAQK4AQHIAQHYAQHoAQLwAQP4AQOIAgE=\"},"
                       "\"phone\": {\"wa_version\": \"2.20.123\",\"mcc\": \"510\",\"mnc\": \"001\",\"os_version\": \"4.4.2\",\"device_manufacturer\": \"HIMAX\",\"device_model\": \"HIMAX\",\"os_build_number\": \"ZOOM\"},"
                       "\"pushname\": \"Netizen\","
                       "\"tos\": 0"
                       "}]";
    ptr = str;
    ZERO(json_parse_object(&ptr));

    ZERO(strcmp(json_get("ref"), "1@N9QMCQb2tmj/JTss5bwEVyDj9Xl1U6uvjlXUnhe0LkmOQ9p761T2CtKvTXAZOcRdEbOVGgiFs37P8Q=="));
    ZERO(strcmp(json_get("wid"), "6285640992980@c.us"));
    ZERO(strcmp(json_get("connected"), "true"));
    ZERO(strcmp(json_get("isResponse"), "true"));
    ZERO(strcmp(json_get("serverToken"), "1@GGJVQhXxUBmXONlj2WS2C52gmG+6h+yG1urYJl4oRKhbtMJTGicetPymkC5Qj9TQ2xQN8Yt/yedUCg=="));
    ZERO(strcmp(json_get("browserToken"), "1@ufhB49yNHOS+2WpsIjlMDL/TrkAM6T+JW8bdwrD2y2ICQGHRD7W5igWiKBF6kAMBKi8kvELJsfGoeig4YY0tjHCJkdsAqoAkuH2/WyeTVPVXwMY6pHTafWBVOYPv/uOIWqYoUvDvk9yU6cRRoLS80Q=="));
    ZERO(strcmp(json_get("clientToken"), "QNZq7et2EO7ZTBOv6quFvC9F4oCPqE/Q22mZLzAktmU="));
    ZERO(strcmp(json_get("lc"), "ID"));
    ZERO(strcmp(json_get("lg"), "in"));
    ZERO(strcmp(json_get("locales"), "in-ID"));
    ZERO(strcmp(json_get("is24h"), "true"));
    ZERO(strcmp(json_get("protoVersion"), "[0,17]"));
    ZERO(strcmp(json_get("binVersion"), "11"));
    ZERO(strcmp(json_get("battery"), "74"));
    ZERO(strcmp(json_get("plugged"), "false"));
    ZERO(strcmp(json_get("platform"), "android"));
    ZERO(strcmp(json_get("features"), "{ \"URL\": true,\"FLAGS\": \"EAEYASgBOAFAAUgBWAFgAWgBeAGYAQGgAQGwAQK4AQHIAQHYAQHoAQLwAQP4AQOIAgE=\"}"));
    ZERO(strcmp(json_get("phone"), "{\"wa_version\": \"2.20.123\",\"mcc\": \"510\",\"mnc\": \"001\",\"os_version\": \"4.4.2\",\"device_manufacturer\": \"HIMAX\",\"device_model\": \"HIMAX\",\"os_build_number\": \"ZOOM\"}"));
    ZERO(strcmp(json_get("pushname"), "Netizen"));
    ZERO(strcmp(json_get("tos"), "0"));

    return 0;
}

int test_main()
{

    return test_json_field() || test_json_parse_init() || test_json_parse_conn() || test_json_parse_conn_takeover();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
