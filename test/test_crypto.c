#include <mbedtls/error.h>
#include <crypto.h>
#include <cfg.h>
#include "test.h"

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

    crypto_free_keys(aliceKeys);
    crypto_free_keys(bobKeys);
    return 0;
}

int test_parse_server_keys(){
    return 0;
}

int test_main()
{

    //return test_keys() || test_random();
    return test_parse_server_keys();
}

int test_setup()
{
    CFG cfg;
    //cfg.client_id = base;
    return crypto_init();
}

int test_cleanup()
{
    return crypto_free();
}
