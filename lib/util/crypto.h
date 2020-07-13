#pragma once
#include <mbedtls/ecdh.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#define CRYPTO_DUMP_MPI(var) crypto_dump_mpi(&var, #var)
#define CRYPTO_DUMP_POINT(var) crypto_dump_point(&var, #var)

typedef struct crypto_keys
{
    /* The private key. */
    mbedtls_mpi d;
    /* The public key. */
    mbedtls_ecp_point Q;
    /* The value of the public key of the peer. */
    // mbedtls_ecp_point Qp;
    /* The shared secret. */
    mbedtls_mpi z;
} crypto_keys;

void crypto_dump_mpi(mbedtls_mpi *mpi, const char *name);
void crypto_dump_point(mbedtls_ecp_point *P, const char *name);
int crypto_init();
int crypto_free();
int crypto_compute_shared(crypto_keys *ctx, mbedtls_ecp_point *theirPublic);
crypto_keys *crypto_gen_keys();
void crypto_free_keys(crypto_keys *ctx);