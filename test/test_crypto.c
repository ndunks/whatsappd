#include <mbedtls/error.h>
#include <crypto.h>
#include "test.h"

int test_keys()
{
    char buf[1024];
    crypto_keys *aliceKeys = crypto_gen_keys(), *bobKeys = crypto_gen_keys();
    TRUTHY(&(aliceKeys->d) != NULL);
    TRUTHY(&(aliceKeys->Q) != NULL);
    TRUTHY(&(bobKeys->d) != NULL);
    TRUTHY(&(bobKeys->Q) != NULL);

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

    CRYPTO_DUMP_MPI(aliceKeys->z);
    CRYPTO_DUMP_MPI(bobKeys->z);

    crypto_free_keys(aliceKeys);
    crypto_free_keys(bobKeys);
    return 0;
}

int test_main()
{

    return test_keys();
}

int test_setup()
{
    return crypto_init();
}

int test_cleanup()
{
    return crypto_free();
}
