#include <malloc.h>

#include "color.h"
#include "crypto.h"

// static enum { crypto_state_free,
//               crypto_state_intialized
// } crypto_state = crypto_state_free;
static mbedtls_ecp_group *grp;
static mbedtls_ctr_drbg_context *ctr_drbg;
static mbedtls_entropy_context *entropy;

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
    warn("todo: crypto_dump_point SegV fix");
    return;
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
    // if (mbedtls_ecdh_setup(ctx, MBEDTLS_ECP_DP_CURVE25519) != 0)
    // {
    //     err("Crypto: Fail gen keys");
    //     free(ctx);
    //     return NULL;
    // }
    // if (mbedtls_ecp_gen_privkey(grp,
    //                             &ctx->d,
    //                             mbedtls_ctr_drbg_random,
    //                             ctr_drbg) != 0)
    // {
    //     err("Crypto: Fail gen private keys");
    //     free(ctx);
    //     return NULL;
    // };
    if (mbedtls_ecdh_gen_public(grp,
                                &ctx->d,
                                &ctx->Q,
                                mbedtls_ctr_drbg_random,
                                ctr_drbg) != 0)
    {
        err("Crypto: Fail gen public keys");
        free(ctx);
        return NULL;
    };
    return ctx;
}

int crypto_compute_shared(crypto_keys *ctx, mbedtls_ecp_point *theirPublic)
{
    return mbedtls_ecdh_compute_shared(&grp,
                                       &ctx->z,
                                       theirPublic,
                                       &ctx->d,
                                       mbedtls_ctr_drbg_random,
                                       &ctr_drbg);
}

void crypto_free_keys(crypto_keys *ctx)
{
    mbedtls_mpi_free(&ctx->d);
    mbedtls_mpi_free(&ctx->z);
    mbedtls_ecp_point_free(&ctx->Q);
    free(ctx);
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

    ctr_drbg = malloc(sizeof(mbedtls_ctr_drbg_context));
    entropy = malloc(sizeof(mbedtls_entropy_context));

    mbedtls_ctr_drbg_init(ctr_drbg);
    mbedtls_entropy_init(entropy);

    if (mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
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
    mbedtls_ctr_drbg_free(ctr_drbg);
    mbedtls_entropy_free(entropy);
    free(grp);
    free(ctr_drbg);
    free(entropy);
    return 0;
}