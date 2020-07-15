#include <mbedtls/error.h>
#include <crypto.h>
#include <cfg.h>

#include "test.h"
#include "data_auth.h"

int test_random()
{
    char buf[10], *rand1 = malloc(32), *null = calloc(32, 1);
    memset(buf, 0x0, 10);
    info("rand: buf");
    hexdump(buf, 10);
    crypto_random(buf, 10);
    info("rand: buf");
    hexdump(buf, 10);
    TRUTHY(memcmp(buf, null, 10) != 0);

    memset(rand1, 0, 10);
    crypto_random(rand1, 10);
    ZERO(rand1 == NULL);
    info("rand: rand1");
    hexdump(rand1, 10);
    TRUTHY(memcmp(rand1, null, 10) != 0);

    memset(rand1, 0, 16);
    crypto_random(rand1, 16);
    ZERO(rand1 == NULL);
    info("rand: rand1");
    hexdump(rand1, 16);
    TRUTHY(memcmp(rand1, null, 16) != 0);

    memset(rand1, 0, 32);
    crypto_random(rand1, 32);
    ZERO(rand1 == NULL);
    info("rand: rand1");
    hexdump(rand1, 32);
    TRUTHY(memcmp(rand1, null, 32) != 0);

    free(rand1);
    free(null);
    return 0;
}

int test_keys()
{
    char buf[1024];
    crypto_keys *aliceKeys = crypto_gen_keys(), *bobKeys = crypto_gen_keys();
    TRUTHY(&(aliceKeys->d) != NULL);
    TRUTHY(&(aliceKeys->Q) != NULL);
    TRUTHY(&(bobKeys->d) != NULL);
    TRUTHY(&(bobKeys->Q) != NULL);

    FALSY(mbedtls_ecp_is_zero(&bobKeys->Q));
    FALSY(mbedtls_ecp_is_zero(&aliceKeys->Q));

    CRYPTO_DUMP_MPI(aliceKeys->d);
    CRYPTO_DUMP_POINT(aliceKeys->Q);
    CRYPTO_DUMP_MPI(bobKeys->d);
    CRYPTO_DUMP_POINT(bobKeys->Q);

    ret_val = crypto_compute_shared(aliceKeys, &bobKeys->Q);
    if (ret_val != 0)
    {
        mbedtls_strerror(ret_val, buf, 1024);
        err("Crypto: %s", buf);
        return ret_val;
    }
    ret_val = crypto_compute_shared(bobKeys, &aliceKeys->Q);
    if (ret_val != 0)
    {
        mbedtls_strerror(ret_val, buf, 1024);
        err("Crypto: %s", buf);
        return ret_val;
    }

    CRYPTO_DUMP_MPI(aliceKeys->z);
    CRYPTO_DUMP_MPI(bobKeys->z);
    ZERO(mbedtls_mpi_cmp_mpi(&aliceKeys->z, &bobKeys->z));

    crypto_keys_free(aliceKeys);
    crypto_keys_free(bobKeys);
    return 0;
}

int test_parse_server_keys()
{
    CFG cfg;
    char null[160],
        *clientId = DATA_AUTH_CLIENT_ID,
        *keys_secret = DATA_AUTH_KEYS_SECRET,
        *keys_private = DATA_AUTH_KEYS_PRIVATE,
        *keys_public = DATA_AUTH_KEYS_PUBLIC,
        *serverSecret = DATA_AUTH_SERVER_SECRET,
        *aesKey = DATA_AUTH_AESKEY,
        *macKey = DATA_AUTH_MACKEY,
        aesKeyByte[32],
        macKeyByte[32];
    // *tokens_client = DATA_AUTH_TOKENS_CLIENT,
    // *tokens_server = DATA_AUTH_TOKENS_SERVER,
    // *tokens_browser = DATA_AUTH_TOKENS_BROWSER;

    memset(&cfg, 0, sizeof(CFG));
    memset(null, 0, 160);

    EQUAL(crypto_base64_decode(cfg.client_id, CFG_CLIENT_ID_LEN, clientId, strlen(clientId)), CFG_CLIENT_ID_LEN);
    EQUAL(crypto_base64_decode(cfg.keys.private, CFG_KEY_LEN, keys_private, strlen(keys_private)), CFG_KEY_LEN);
    EQUAL(crypto_base64_decode(cfg.keys.public, CFG_KEY_LEN, keys_public, strlen(keys_public)), CFG_KEY_LEN);
    EQUAL(crypto_base64_decode(cfg.keys.secret, CFG_KEY_LEN, keys_secret, strlen(keys_secret)), CFG_KEY_LEN);

    ZERO(crypto_parse_server_keys(serverSecret, strlen(serverSecret), &cfg));

    TRUTHY(memcmp(cfg.serverSecret, null, CFG_SERVER_SECRET_LEN) != 0);
    TRUTHY(memcmp(cfg.aesKey, null, CFG_KEY_LEN) != 0);
    TRUTHY(memcmp(cfg.macKey, null, CFG_KEY_LEN) != 0);

    EQUAL(crypto_base64_decode(aesKeyByte, CFG_KEY_LEN, aesKey, strlen(aesKey)), CFG_KEY_LEN);
    EQUAL(crypto_base64_decode(macKeyByte, CFG_KEY_LEN, macKey, strlen(macKey)), CFG_KEY_LEN);

    ZERO(memcmp(cfg.aesKey, aesKeyByte, CFG_KEY_LEN));
    ZERO(memcmp(cfg.macKey, macKeyByte, CFG_KEY_LEN));

    return 0;
}

int test_main()
{

    return test_keys() || test_random() || test_parse_server_keys();
}

int test_setup()
{
    return crypto_init();
}

int test_cleanup()
{
    return crypto_free();
}
