#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <cfg.h>
#include <helper.h>
#include <mbedtls/base64.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>

#include "data_auth.h"
#include "test.h"

#define COMPARE_CFG                                                    \
    do                                                                 \
    {                                                                  \
        TRUTHY(cfg1->cfg_file_version == cfg2->cfg_file_version);      \
        ZERO(memcmp(cfg1->client_id, cfg2->client_id, 16));            \
        ZERO(memcmp(cfg1->keys.private, cfg2->keys.private, 32));      \
        ZERO(memcmp(cfg1->keys.public, cfg2->keys.public, 32));        \
        ZERO(memcmp(cfg1->tokens.client, cfg2->tokens.client, 60));    \
        ZERO(memcmp(cfg1->tokens.server, cfg2->tokens.server, 120));   \
        ZERO(memcmp(cfg1->tokens.browser, cfg2->tokens.browser, 180)); \
        ZERO(strcmp(cfg1->tokens.client, cfg2->tokens.client));        \
        ZERO(strcmp(cfg1->tokens.server, cfg2->tokens.server));        \
        ZERO(strcmp(cfg1->tokens.browser, cfg2->tokens.browser));      \
    } while (0)

static CFG *cfg1, *cfg2;

int base64_decode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;

    return mbedtls_base64_decode(
        (unsigned char *)dst,
        dst_len,
        &written,
        (const unsigned char *)src,
        src_len);
}

void simple_random_bytes(char *bytes, size_t size)
{
    srand(time(0) + size + rand());
    while (size--)
        bytes[size] = rand();
}

int test_config_file()
{
    char buf[255] = {0}, *homedir;

    EQUAL(cfg_file("/sbin/asdfg"), -1);
    EQUAL(cfg_file("/etc/passwd"), -1);
    ZERO(cfg_file("/tmp/nothing.cfg"));
    ZERO(strcmp("/tmp/nothing.cfg", cfg_file_get()));
    // Default is in home
    TRUTHY(cfg_file(NULL) >= 0);
    if ((homedir = getenv("HOME")) == NULL)
    {
        homedir = getpwuid(getuid())->pw_dir;
    }
    strcat(buf, homedir);
    strcat(buf, "/.whatsappd.cfg");

    ZERO(strcmp(buf, cfg_file_get()));

    return 0;
}

int test_random_values()
{
    TRUTHY(cfg_file("tmp/whatsappd.cfg") >= 0);
    info("CFGFILE: %s", cfg_file_get());

    cfg1->cfg_file_version = 1;

    simple_random_bytes(cfg1->client_id, 16);
    ZERO(cfg_save(cfg1));
    ZERO(cfg_load(cfg2));

    TRUTHY(cfg1->cfg_file_version == cfg2->cfg_file_version);
    ZERO(memcmp(cfg1->client_id, cfg2->client_id, 16));

    memset(cfg2, 0, sizeof(CFG));

    simple_random_bytes(cfg1->keys.private, 32);
    simple_random_bytes(cfg1->keys.public, 32);
    simple_random_bytes(cfg1->serverSecret, 144);

    strcpy(cfg1->tokens.client, "%#^$#%^$#&$^#&$*#&$#)(*");
    strcpy(cfg1->tokens.server, "636tgyf7ryfhr7ryhf7rufhrg8rgrgj8rgj");
    strcpy(cfg1->tokens.browser, "||ASDFGHJKDFKDFKDKFKDFKFDK636tgyf7ryfhr7ryhf7rufhrg8rgrgj8rgj%#^$#%^$#&$^#&$*#&$#)(*||");

    ZERO(cfg_save(cfg1));
    ZERO(cfg_load(cfg2));

    COMPARE_CFG;

    return 0;
}

int test_real_values()
{
    // Working config from WaJs .auth file
    const char
        *clientId = DATA_AUTH_CLIENT_ID,
        *keys_private = DATA_AUTH_KEYS_PRIVATE,
        *keys_public = DATA_AUTH_KEYS_PUBLIC,
        *serverSecret = DATA_AUTH_SERVER_SECRET,
        *tokens_client = DATA_AUTH_TOKENS_CLIENT,
        *tokens_server = DATA_AUTH_TOKENS_SERVER,
        *tokens_browser = DATA_AUTH_TOKENS_BROWSER;

    TRUTHY(cfg_file("tmp/whatsappd.cfg") >= 0);
    info("CFGFILE: %s", cfg_file_get());

    memset(cfg1, 0, sizeof(CFG));
    memset(cfg2, 0, sizeof(CFG));

    cfg1->cfg_file_version = 1;

    FALSY(cfg_has_credentials(cfg1));
    FALSY(cfg_has_credentials(cfg2));

    ZERO(base64_decode(cfg1->client_id, 16, clientId, strlen(clientId)));
    FALSY(cfg_has_credentials(cfg1));
    ZERO(base64_decode(cfg1->keys.private, 32, keys_private, strlen(keys_private)));
    FALSY(cfg_has_credentials(cfg1));
    ZERO(base64_decode(cfg1->keys.public, 32, keys_public, strlen(keys_public)));
    FALSY(cfg_has_credentials(cfg1));
    ZERO(base64_decode(cfg1->serverSecret, 144, serverSecret, strlen(serverSecret)));
    FALSY(cfg_has_credentials(cfg1));

    strcpy(cfg1->tokens.client, tokens_client);
    FALSY(cfg_has_credentials(cfg1));
    strcpy(cfg1->tokens.server, tokens_server);
    FALSY(cfg_has_credentials(cfg1));
    strcpy(cfg1->tokens.browser, tokens_browser);
    TRUTHY(cfg_has_credentials(cfg1));

    ZERO(cfg_save(cfg1));
    memset(cfg1, 0, sizeof(CFG));
    ZERO(cfg_load(cfg1));
    ZERO(cfg_load(cfg2));

    TRUTHY(cfg_has_credentials(cfg1));
    TRUTHY(cfg_has_credentials(cfg2));

    COMPARE_CFG;

    return 0;
}

int test_main()
{

    return test_config_file() || test_random_values() || test_real_values();
}

int test_setup()
{
    cfg1 = calloc(sizeof(CFG), 1);
    cfg2 = calloc(sizeof(CFG), 1);
    info("cfg size: %lu", sizeof(CFG));
    return 0;
}

int test_cleanup()
{
    free(cfg1);
    free(cfg2);
    return 0;
}
