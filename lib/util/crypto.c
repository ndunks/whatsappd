#include <malloc.h>
#include <cfg.h>

#include "color.h"
#include "crypto.h"

static mbedtls_ecp_group *grp;

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

crypto_keys *crypto_gen_keys()
{
    crypto_keys *ctx = calloc(sizeof(crypto_keys), 1);
    mbedtls_mpi_init(&ctx->d);
    mbedtls_mpi_init(&ctx->z);
    mbedtls_ecp_point_init(&ctx->Q);

    // file://./../mbedtls/tests/suites/test_suite_ecdh.function

    if (mbedtls_ecdh_gen_public(grp,
                                &ctx->d,
                                &ctx->Q,
                                mbedtls_ctr_drbg_random,
                                crypto_p_rng) != 0)
    {
        err("Crypto: Fail gen public keys");
        free(ctx);
        return NULL;
    };
    return ctx;
}

int crypto_random(char *buf, size_t len)
{
    return mbedtls_ctr_drbg_random(crypto_p_rng, (unsigned char *)buf, len);
}

int crypto_compute_shared(crypto_keys *ctx, mbedtls_ecp_point *theirPublic)
{
    return mbedtls_ecdh_compute_shared(grp,
                                       &ctx->z,
                                       theirPublic,
                                       &ctx->d,
                                       mbedtls_ctr_drbg_random,
                                       crypto_p_rng);
}

void crypto_free_keys(crypto_keys *ctx)
{
    mbedtls_mpi_free(&ctx->d);
    mbedtls_mpi_free(&ctx->z);
    mbedtls_ecp_point_free(&ctx->Q);
    free(ctx);
}

size_t crypto_base64_encode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;

    mbedtls_base64_encode(
        (unsigned char *)dst,
        dst_len - 1,
        &written,
        (const unsigned char *)src,
        src_len);
    dst[written] = 0;

    return written;
}

size_t crypto_base64_decode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;

    mbedtls_base64_decode(
        (unsigned char *)dst,
        dst_len - 1,
        &written,
        (const unsigned char *)src,
        src_len);
    dst[written] = 0;

    return written;
}

int crypto_init()
{
    char pers[] = "whatsappd";
    grp = malloc(sizeof(mbedtls_ecp_group));

    mbedtls_ecp_group_init(grp);

    if (mbedtls_ecp_group_load(grp, MBEDTLS_ECP_DP_CURVE25519) != 0)
    {
        err("Crypto: Fail load CURVE25519");
        mbedtls_ecp_group_free(grp);
        return 1;
    }

    crypto_p_rng = malloc(sizeof(mbedtls_ctr_drbg_context));
    crypto_entropy = malloc(sizeof(mbedtls_entropy_context));

    mbedtls_ctr_drbg_init(crypto_p_rng);
    mbedtls_entropy_init(crypto_entropy);

    if (mbedtls_ctr_drbg_seed(crypto_p_rng,
                              mbedtls_entropy_func,
                              crypto_entropy,
                              (const unsigned char *)pers,
                              sizeof pers) != 0)
    {
        crypto_free();
        err("Crypto: Fail seed entropy)");
        return 1;
    }

    return 0;
}

int crypto_free()
{
    mbedtls_ecp_group_free(grp);
    mbedtls_ctr_drbg_free(crypto_p_rng);
    mbedtls_entropy_free(crypto_entropy);
    free(grp);
    free(crypto_p_rng);
    free(crypto_entropy);
    return 0;
}