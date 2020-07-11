#include <helper.h>
#include "test.h"

static int test_random_bytes()
{
    char *rand1 = helper_random_bytes(10);
    ZERO(rand1 == NULL);
    free(rand1);

    rand1 = helper_random_bytes(16);
    ZERO(rand1 == NULL);
    free(rand1);

    rand1 = helper_random_bytes(32);
    ZERO(rand1 == NULL);
    free(rand1);

    return 0;
}

static int test_config_default()
{
    Config cfg1;
    info("Config size %lu byes", sizeof(Config));
    memset(&cfg1, 0, sizeof(cfg1));

    TRUTHY(cfg1.client_id == NULL);

    TRUTHY(cfg1.keys.secret == NULL);
    TRUTHY(cfg1.keys.private == NULL);
    TRUTHY(cfg1.keys.public == NULL);

    TRUTHY(cfg1.tokens.client == NULL);
    TRUTHY(cfg1.tokens.server == NULL);
    TRUTHY(cfg1.tokens.browser == NULL);

    TRUTHY(cfg1.serverSecret == NULL);
    TRUTHY(cfg1.aesKey == NULL);
    TRUTHY(cfg1.macKey == NULL);
    helper_config_init_default(&cfg1);
    FALSY(cfg1.client_id == NULL);
    info("client-id (%lu) %s", strlen(cfg1.client_id), cfg1.client_id);
    TRUTHY(strlen(cfg1.client_id) > 0);

    FALSY(cfg1.keys.secret == NULL);
    FALSY(cfg1.keys.private == NULL);
    FALSY(cfg1.keys.public == NULL);

    return 0;
}

static int test_base64()
{
    const char *str = "Hello World", *str_b64 = "SGVsbG8gV29ybGQ=";
    char buf[255];
    helper_base64_encode(buf, 255, str, strlen(str));
    info("b64: %s", buf);
    ZERO(strcmp(buf, str_b64));

    helper_base64_decode(buf, 255, str_b64, strlen(str_b64));
    info("str: %s", buf);
    ZERO(strcmp(buf, str));
    return 0;
}

int test_main()
{

    //return test_base64() || test_random_bytes() || test_config_default();
    return test_config_default();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
