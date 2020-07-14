#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <cfg.h>
#include <helper.h>
#include <mbedtls/base64.h>

#include "test.h"

#define COMPARE_CFG                                                    \
    do                                                                 \
    {                                                                  \
        TRUTHY(cfg1->cfg_file_version == cfg2->cfg_file_version);      \
        ZERO(memcmp(cfg1->client_id, cfg2->client_id, 16));            \
        ZERO(memcmp(cfg1->keys.private, cfg2->keys.private, 32));      \
        ZERO(memcmp(cfg1->keys.public, cfg2->keys.public, 32));        \
        ZERO(memcmp(cfg1->keys.secret, cfg2->keys.secret, 32));        \
        ZERO(memcmp(cfg1->aesKey, cfg2->aesKey, 32));                  \
        ZERO(memcmp(cfg1->macKey, cfg2->macKey, 32));                  \
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

int test_random_values()
{

    cfg1->cfg_file_version = 0x99;

    simple_random_bytes(cfg1->client_id, 16);
    ZERO(cfg_save(cfg1));
    ZERO(cfg_load(cfg2));

    TRUTHY(cfg1->cfg_file_version == cfg2->cfg_file_version);
    ZERO(memcmp(cfg1->client_id, cfg2->client_id, 16));

    memset(cfg2, 0, sizeof(CFG));

    simple_random_bytes(cfg1->keys.private, 32);
    simple_random_bytes(cfg1->keys.public, 32);
    simple_random_bytes(cfg1->keys.secret, 32);
    simple_random_bytes(cfg1->serverSecret, 144);
    simple_random_bytes(cfg1->aesKey, 32);
    simple_random_bytes(cfg1->macKey, 32);

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
        *clientId = "5TPN1PAqE1xAubOU6pf87Q==",
        *keys_secret = "uewWdPmaNsOOXI/zZoSKprXvZ6mgi/KfWCBnWB6cxnk=",
        *keys_private = "uOwWdPmaNsOOXI/zZoSKprXvZ6mgi/KfWCBnWB6cxnk=",
        *keys_public = "WSFfqJagGZiSBXVBR2RgaOQc0Qhp530z3pDRjhwN2hQ=",
        *serverSecret = "mzpO/Ue00iOHP+HF+Cqq9tBjL6ecYuKy8aPrAefIoHI6Lf2Nqf4ggb5iv5dMxzXNj+ctY/bkEpMAeJEWbPDiOBB8iEKU0kQ/DQpdNd0p8G+kW5I92u5Vye/CgqnYl8di/6ZRJiMQp87SjuPDYkSghUxBpOtPbuLGsL2iiqLF/3iM3gY1JU7RDQuWcmZQkt1C",
        *aesKey = "/RN1S3CZHT7FJnx7yI+Z8FDEkUBenv2W2XD3lGZqL4c=",
        *macKey = "8o1GyJN9XgjnsKAOfUqJdjO71/7wLKZVK+Th9fk3X0w=",
        *tokens_client = "7sujfCM4+5dyDbGefsbRV1qcHPRDXevYdpTYmFdc4Zg=",
        *tokens_server = "1@on5QYjmA9bxWFoCDXucNCfQxS7QUDfSWuOrSaK5rvwOn/tP0AW1CpXy/RzZPoWpSK1EPPrZfCGC9ng==",
        *tokens_browser = "1@H8GC1RsE0bduCJjaIyZmNT96j2RdAAXEJ7e0l2QUwxQrrVDlghGJdiSU3731iFW+1PVX4+hQ627mazZu3vPHSfkC765o/wqicsMWG7zZ2Jf/4pnkSHuR/264qACPz6NlNmXMxLacwxwYX5/ct4YZKA==";

    memset(cfg1, 0, sizeof(CFG));
    memset(cfg2, 0, sizeof(CFG));

    cfg1->cfg_file_version = 1;

    ZERO(base64_decode(cfg1->client_id, 16, clientId, strlen(clientId)));
    ZERO(base64_decode(cfg1->keys.private, 32, keys_private, strlen(keys_private)));
    ZERO(base64_decode(cfg1->keys.public, 32, keys_public, strlen(keys_public)));
    ZERO(base64_decode(cfg1->keys.secret, 32, keys_secret, strlen(keys_secret)));
    ZERO(base64_decode(cfg1->serverSecret, 144, serverSecret, strlen(serverSecret)));
    ZERO(base64_decode(cfg1->aesKey, 32, aesKey, strlen(aesKey)));
    ZERO(base64_decode(cfg1->macKey, 32, macKey, strlen(macKey)));

    strcpy(cfg1->tokens.client, tokens_client);
    strcpy(cfg1->tokens.server, tokens_server);
    strcpy(cfg1->tokens.browser, tokens_browser);

    ZERO(cfg_save(cfg1));
    ZERO(cfg_load(cfg2));

    COMPARE_CFG;

    return 0;
}

int test_main()
{

    return test_random_values() || test_real_values();
}

int test_setup()
{
    char buf[255];
    cfg_config_file = "tmp/whatsappd.cfg";
    getcwd(buf, 255);
    info("CFGFILE: %s/%s", buf, cfg_config_file);

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
