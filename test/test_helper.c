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

static int test_config()
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
    
    return 0;
}

int test_main()
{

    return test_random_bytes() || test_config();
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
