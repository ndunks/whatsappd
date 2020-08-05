#include "crypto.h"

void crypto_dump_mpi(mbedtls_mpi *mpi, const char *name)
{
    unsigned char buf[1024];
    size_t size = mpi->n * sizeof(mbedtls_mpi_uint);

    if (mbedtls_mpi_write_binary_le(mpi, buf, 1024) != 0)
    {
        err("mbedtls_mpi_write_binary");
        return;
    }
    info("%s (%lu bytes):", name, size);
    hexdump(buf, size);
}

void crypto_dump_point(mbedtls_ecp_point *P, const char *name)
{
    unsigned char buf[1024];
    size_t size = 0;
    // MBEDTLS_ECDH_VARIANT_MBEDTLS_2_0
    mbedtls_ecp_point_write_binary(grp, P, MBEDTLS_ECP_PF_UNCOMPRESSED,
                                   &size, buf, 1024);

    info("%s (%lu bytes):", name, size);
    hexdump(buf, size);
}

/* Generate private and public keys */
crypto_keys *crypto_gen_keys()
{
    crypto_keys *ctx = crypto_keys_init(NULL, NULL);

    // file://./../mbedtls/tests/suites/test_suite_ecdh.function

    if (mbedtls_ecdh_gen_public(grp,
                                &ctx->d,
                                &ctx->Q,
                                mbedtls_ctr_drbg_random,
                                crypto_p_rng) != 0)
    {
        err("Crypto: Fail gen public keys");
        crypto_keys_free(ctx);
        return NULL;
    };
    return ctx;
}
