#include <mbedtls/error.h>
#include <crypto.h>
#include <util.h>

#include "test.h"
#include "data_auth.h"

int test_random()
{
    char buf[10], *rand1 = malloc(32), *null = calloc(32, 1);
    memset(buf, 0x0, 10);
    // info("rand: buf");
    // hexdump(buf, 10);
    ZERO(crypto_random(buf, 10));
    // info("rand: buf");
    // hexdump(buf, 10);
    TRUTHY(memcmp(buf, null, 10) != 0);

    memset(rand1, 0, 32);
    ZERO(crypto_random(rand1, 32));
    // info("rand: rand1");
    // hexdump(rand1, 32);
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

    // Shared secret must be equal
    ZERO(mbedtls_mpi_cmp_mpi(&aliceKeys->z, &bobKeys->z));

    crypto_keys_free(aliceKeys);
    crypto_keys_free(bobKeys);
    return 0;
}

int test_crypto_cfg()
{
    CFG cfg, *cfg2;
    crypto_keys *keys, *keys2;
    char null_keys[CFG_KEY_LEN];

    memset(&cfg, 0, sizeof(CFG));
    memset(null_keys, 0, CFG_KEY_LEN);
    cfg2 = &cfg;

    keys = crypto_gen_keys();
    crypto_keys_store_cfg(keys, cfg2);
    keys2 = crypto_keys_init(cfg.keys.private, cfg.keys.public);
    FALSY(keys2 == NULL);
    ZERO(mbedtls_mpi_cmp_mpi(&keys->d, &keys2->d));
    ZERO(mbedtls_ecp_point_cmp(&keys->Q, &keys2->Q));

    crypto_keys_free(keys);
    crypto_keys_free(keys2);

    FALSY(cfg.keys.private == NULL);
    FALSY(cfg.keys.public == NULL);

    TRUTHY(memcmp(null_keys, cfg.keys.private, CFG_KEY_LEN));
    TRUTHY(memcmp(null_keys, cfg.keys.public, CFG_KEY_LEN));
    ok("test_crypto_cfg OK");

    return 0;
}

int test_parse_server_keys()
{
    CFG cfg;
    char null[160],
        *clientId = DATA_AUTH_CLIENT_ID,
        *keys_private = DATA_AUTH_KEYS_PRIVATE,
        *keys_public = DATA_AUTH_KEYS_PUBLIC,
        *serverSecret = DATA_AUTH_SERVER_SECRET,
        *aesKey = DATA_AUTH_AESKEY,
        *macKey = DATA_AUTH_MACKEY,
        server_secret_bytes[144],
        aesKeyByte[32],
        macKeyByte[32];

    memset(&cfg, 0, sizeof(CFG));
    memset(null, 0, 160);

    EQUAL(crypto_base64_decode(cfg.client_id, CFG_CLIENT_ID_LEN, clientId, strlen(clientId)), CFG_CLIENT_ID_LEN);
    EQUAL(crypto_base64_decode(cfg.keys.private, CFG_KEY_LEN, keys_private, strlen(keys_private)), CFG_KEY_LEN);
    EQUAL(crypto_base64_decode(cfg.keys.public, CFG_KEY_LEN, keys_public, strlen(keys_public)), CFG_KEY_LEN);
    EQUAL(crypto_base64_decode(server_secret_bytes, CFG_SERVER_SECRET_LEN, serverSecret, strlen(serverSecret)), CFG_SERVER_SECRET_LEN);

    ZERO(crypto_parse_server_keys(server_secret_bytes, &cfg));

    TRUTHY(memcmp(cfg.serverSecret, null, CFG_SERVER_SECRET_LEN) != 0);

    EQUAL(crypto_base64_decode(aesKeyByte, CFG_KEY_LEN, aesKey, strlen(aesKey)), CFG_KEY_LEN);
    EQUAL(crypto_base64_decode(macKeyByte, CFG_KEY_LEN, macKey, strlen(macKey)), CFG_KEY_LEN);

    ZERO(memcmp(crypto_aes_keys.enc, aesKeyByte, CFG_KEY_LEN));
    ZERO(memcmp(crypto_aes_keys.mac, macKeyByte, CFG_KEY_LEN));
    ok("test_parse_server_keys OK");
    return 0;
}

int test_main()
{

    return test_keys() || test_random() || test_crypto_cfg() || test_parse_server_keys();
}

int test_setup()
{
    return crypto_init();
}

int test_cleanup()
{
    crypto_free();
    return 0;
}
