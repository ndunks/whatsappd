#include <malloc.h>
#include <string.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/hkdf.h>
#include <mbedtls/sha256.h>
#include "mbedtls/aes.h"

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

crypto_keys *crypto_keys_init(const char *private_key, const char *public_key)
{
    crypto_keys *ctx = calloc(sizeof(crypto_keys), 1);
    mbedtls_mpi_init(&ctx->d);
    mbedtls_mpi_init(&ctx->z);
    mbedtls_ecp_point_init(&ctx->Q);

    if (private_key != NULL)
    {
        TRY(mbedtls_mpi_read_binary_le(
            &ctx->d,
            (const unsigned char *)private_key,
            CFG_KEY_LEN));
    }

    if (public_key != NULL)
    {
        TRY(mbedtls_ecp_point_read_binary(
            grp,
            &ctx->Q,
            (const unsigned char *)public_key,
            CFG_KEY_LEN));
    }

    return ctx;
CATCH:
    crypto_keys_free(ctx);
    return NULL;
}

void crypto_keys_free(crypto_keys *ctx)
{
    mbedtls_mpi_free(&ctx->d);
    mbedtls_mpi_free(&ctx->z);
    mbedtls_ecp_point_free(&ctx->Q);
    free(ctx);
}

/** return positif int when OK **/
size_t crypto_base64_encode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;
    int ret;

    ret = mbedtls_base64_encode(
        (unsigned char *)dst,
        dst_len - 1,
        &written,
        (const unsigned char *)src,
        src_len);

    if (ret != 0)
        return ret;

    dst[written] = 0;

    return written;
}

/** return positif int when OK **/
size_t crypto_base64_decode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    size_t written;
    int ret;

    ret = mbedtls_base64_decode(
        (unsigned char *)dst,
        dst_len,
        &written,
        (const unsigned char *)src,
        src_len);

    if (ret != 0)
        return ret;

    return written;
}

int crypto_parse_server_keys(const char *base64, size_t base64_len, CFG *cfg)
{
    crypto_keys *my_keys = crypto_keys_init(cfg->keys.private, cfg->keys.public);
    mbedtls_ecp_point server_public;
    const mbedtls_md_info_t *md_sha256 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    mbedtls_ecp_point_init(&server_public);

    if (CFG_SERVER_SECRET_LEN != crypto_base64_decode(
                                     cfg->serverSecret,
                                     CFG_SERVER_SECRET_LEN,
                                     base64,
                                     base64_len))
    {
        err("Fail crypto_parse_server_keys");
        CATCH_RET = 1;
        goto CATCH;
    }

    TRY(mbedtls_ecp_point_read_binary(
        grp,
        &server_public,
        (unsigned char *)cfg->serverSecret,
        CFG_KEY_LEN))

    // Shared key
    TRY(crypto_compute_shared(my_keys, &server_public));

    unsigned char key[32],
        shared_secret[32],
        shared_secret_hmac[32],
        validate_secret[144 - 32],
        expanded[80],
        *aes_encrypted,
        *iv,
        decrypted[80] = {0};

    mbedtls_aes_context aes_ctx;

    // Expand HKDF
    TRY(mbedtls_mpi_write_binary_le(&my_keys->z, shared_secret, 32));
    TRY(mbedtls_md_hmac(md_sha256, NULL, 0, shared_secret, 32, key));
    TRY(mbedtls_hkdf_expand(md_sha256, key, 32, NULL, 0, expanded, 80));

    // Validating our key
    memcpy(validate_secret, cfg->serverSecret, 32);
    memcpy(validate_secret + 32, cfg->serverSecret + 64, 144 - 64);
    TRY(mbedtls_md_hmac(
        md_sha256,
        expanded + 32,
        32,
        validate_secret,
        144 - 32,
        shared_secret_hmac));

    if (memcmp(shared_secret_hmac, cfg->serverSecret + 32, 32) != 0)
    {
        CATCH_RET = 1;
        err("Key validation fail");
        goto CATCH;
    }

    // Decrypt main keys
    iv = (uint8_t *)&expanded[64];                     // used 16 byte
    aes_encrypted = (uint8_t *)&cfg->serverSecret[64]; // used len 144 - 64

    mbedtls_aes_init(&aes_ctx);

    TRY(mbedtls_aes_setkey_dec(&aes_ctx, expanded, 32 * 8));
    TRY(mbedtls_aes_crypt_cbc(
        &aes_ctx,
        MBEDTLS_AES_DECRYPT,
        144 - 64,
        iv,
        aes_encrypted,
        decrypted));

    mbedtls_aes_free(&aes_ctx);

    // Fill the result
    memcpy(cfg->aesKey, decrypted, 32);
    memcpy(cfg->macKey, decrypted + 32, 32);

    CATCH_RET = 0;

CATCH:
    crypto_keys_free(my_keys);
    mbedtls_ecp_point_free(&server_public);
    return CATCH_RET;
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

void crypto_free()
{
    mbedtls_ecp_group_free(grp);
    mbedtls_ctr_drbg_free(crypto_p_rng);
    mbedtls_entropy_free(crypto_entropy);
    free(grp);
    free(crypto_p_rng);
    free(crypto_entropy);
}