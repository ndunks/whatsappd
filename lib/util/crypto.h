#pragma once
#include <mbedtls/ecdh.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#include "cfg.h"

#define CRYPTO_DUMP_MPI(var) crypto_dump_mpi(&var, #var)
#define CRYPTO_DUMP_POINT(var) crypto_dump_point(&var, #var)

mbedtls_ctr_drbg_context *crypto_p_rng;
mbedtls_entropy_context *crypto_entropy;

typedef struct aes_keys
{
    char enc[32], mac[32];
} aes_keys;

extern aes_keys crypto_aes_keys;

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

int crypto_init();
void crypto_free();
int crypto_random(char *buf, size_t len);

size_t crypto_base64_encode(char *dst, size_t dst_len, const char *src, size_t src_len);
size_t crypto_base64_decode(char *dst, size_t dst_len, const char *src, size_t src_len);

int crypto_parse_server_keys(const char server_secret[144], CFG *cfg);
int crypto_compute_shared(crypto_keys *ctx, mbedtls_ecp_point *theirPublic);

crypto_keys *crypto_gen_keys();
crypto_keys *crypto_keys_init(const char *private_key, const char *public_key);
int crypto_keys_store_cfg(crypto_keys *keys, CFG *cfg);

void crypto_keys_free(crypto_keys *ctx);
void crypto_dump_mpi(mbedtls_mpi *mpi, const char *name);
void crypto_dump_point(mbedtls_ecp_point *P, const char *name);