#include <malloc.h>
#include <string.h>

#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/hkdf.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>
#include <mbedtls/error.h>

#include "crypto.h"

mbedtls_ctr_drbg_context *crypto_p_rng;
mbedtls_entropy_context *crypto_entropy;
aes_keys crypto_aes_keys;
mbedtls_ecp_group *grp;

static mbedtls_aes_context *crypto_aes_dec_ctx, *crypto_aes_enc_ctx;
static const mbedtls_md_info_t *md_sha256;

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
    char err_buf[256];
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
    if (CATCH_RET < 0)
    {
        mbedtls_strerror(CATCH_RET, err_buf, 256);
        warn("%s", err_buf);
    }
    return NULL;
}

int crypto_keys_store_cfg(crypto_keys *keys, CFG *cfg)
{
    size_t public_key_size;

    TRY(mbedtls_mpi_size(&keys->d) <= 0); // Private key
    TRY(mbedtls_ecp_is_zero(&keys->Q));   // Public yey

    mbedtls_mpi_write_binary_le(&keys->d, (uint8_t *)cfg->keys.private, CFG_KEY_LEN);
    mbedtls_ecp_point_write_binary(
        grp, &keys->Q,
        MBEDTLS_ECP_PF_UNCOMPRESSED,
        &public_key_size,
        (uint8_t *)cfg->keys.public,
        CFG_KEY_LEN);
    TRY(public_key_size <= 0);

CATCH:
    return CATCH_RET;
}

void crypto_keys_free(crypto_keys *ctx)
{
    mbedtls_mpi_free(&ctx->d);
    mbedtls_mpi_free(&ctx->z);
    mbedtls_ecp_point_free(&ctx->Q);
    free(ctx);
}

int crypto_sign(char *dst, char *src, size_t len)
{
    return mbedtls_md_hmac(
        md_sha256,
        (uint8_t *)crypto_aes_keys.mac,
        32,
        (uint8_t *)src,
        len,
        (uint8_t *)dst);
}

int crypto_padding(char *buf, size_t *len)
{
    uint8_t pad, pad_len = 16 - (*len % 16);
    pad = pad_len;

    while (pad_len-- > 0)
    {
        buf[(*len)++] = pad;
    }

    return 0;
}

int crypto_unpadding(char *buf, size_t *len)
{
    int pad_len = buf[*len - 1];
    if (pad_len > 16)
    {
        warn("Invalid pad len: %d", pad_len);
        return 1;
    }
    *len -= pad_len;
    buf[*len] = 0;
    return 0;
}

// Input buffer must be enough to store 16 byte padding
int crypto_encrypt_hmac(char *input, size_t input_len, char *output, size_t *output_len)
{
    uint8_t iv[16];
    crypto_random((char *)iv, 16);

    // padded, input_len adjusted
    crypto_padding(input, &input_len);
    *output_len = input_len + 32 + 16;

    // store IV befor it changed by mbedtls encryption
    memcpy(output + 32, iv, 16);

    CHECK(mbedtls_aes_crypt_cbc(
        crypto_aes_enc_ctx,
        MBEDTLS_AES_ENCRYPT,
        input_len,
        iv,
        (uint8_t *)input,
        (uint8_t *)output + 32 + 16));

    CHECK(mbedtls_md_hmac(
        md_sha256,
        (uint8_t *)crypto_aes_keys.mac,
        32,
        (uint8_t *)output + 32,
        input_len + 16,
        (uint8_t *)output));

    return 0;
}

int crypto_decrypt_hmac(const char *const input, size_t input_len, char *output, size_t *output_len)
{
    uint8_t hmac_check[32], sign[32], iv[16];
    size_t check_len, encrypted_len;

    if (input_len <= 64)
    {
        err("crypto_decrypt: to short");
        return 1;
    }

    memcpy(sign, input, 32);
    memcpy(iv, input + 32, 16);
    check_len = input_len - 32;            // hmac sign
    encrypted_len = input_len - (32 + 16); // iv

    CHECK(mbedtls_md_hmac(
        md_sha256,
        (uint8_t *)crypto_aes_keys.mac,
        32,
        (uint8_t *)input + 32,
        check_len,
        hmac_check));

    if (memcmp(sign, hmac_check, 32) != 0)
    {
        hexdump(sign, 32);
        hexdump(hmac_check, 32);
        err("HMAC Not match!");
        return 1;
    }

    CHECK(mbedtls_aes_crypt_cbc(
        crypto_aes_dec_ctx,
        MBEDTLS_AES_DECRYPT,
        encrypted_len,
        iv,
        (uint8_t *)input + (32 + 16),
        (uint8_t *)output));

    // Padding
    *output_len = encrypted_len;
    crypto_unpadding(output, output_len);
    return 0;
}

int crypto_parse_server_keys(const char server_secret[144], CFG *cfg)
{
    unsigned char key[32],
        shared_secret[32],
        shared_secret_hmac[32],
        validate_secret[144 - 32],
        expanded[80],
        *aes_encrypted,
        *iv,
        decrypted[80] = {0};
    crypto_keys *my_keys = crypto_keys_init(cfg->keys.private, cfg->keys.public);
    mbedtls_ecp_point server_public;

    mbedtls_ecp_point_init(&server_public);

    TRY(mbedtls_ecp_point_read_binary(
        grp,
        &server_public,
        (unsigned char *)server_secret,
        CFG_KEY_LEN));

    // Shared key
    TRY(crypto_compute_shared(my_keys, &server_public));

    mbedtls_aes_context aes_ctx;

    // Expand HKDF
    TRY(mbedtls_mpi_write_binary_le(&my_keys->z, shared_secret, 32));
    TRY(mbedtls_md_hmac(md_sha256, NULL, 0, shared_secret, 32, key));
    TRY(mbedtls_hkdf_expand(md_sha256, key, 32, NULL, 0, expanded, 80));

    // Validating our key
    memcpy(validate_secret, server_secret, 32);
    memcpy(validate_secret + 32, server_secret + 64, 144 - 64);
    TRY(mbedtls_md_hmac(
        md_sha256,
        expanded + 32,
        32,
        validate_secret,
        144 - 32,
        shared_secret_hmac));

    if (memcmp(shared_secret_hmac, server_secret + 32, 32) != 0)
    {
        CATCH_RET = 1;
        err("Key validation fail");
        goto CATCH;
    }

    // Decrypt main keys
    iv = (uint8_t *)&expanded[64];                 // used 16 byte
    aes_encrypted = (uint8_t *)&server_secret[64]; // used len 144 - 64

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

    // store it if null
    if (cfg->serverSecret[0] == 0)
    {
        memcpy(cfg->serverSecret, server_secret, CFG_SERVER_SECRET_LEN);
        info("server_secret saved to CFG");
    }

    // Fill the result
    memcpy(crypto_aes_keys.enc, decrypted, 32);
    memcpy(crypto_aes_keys.mac, decrypted + 32, 32);
    mbedtls_aes_setkey_dec(crypto_aes_dec_ctx, (uint8_t *)crypto_aes_keys.enc, 32 * 8);
    mbedtls_aes_setkey_enc(crypto_aes_enc_ctx, (uint8_t *)crypto_aes_keys.enc, 32 * 8);

    info("Gained aes & mac key");

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

    md_sha256 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    crypto_p_rng = malloc(sizeof(mbedtls_ctr_drbg_context));
    crypto_entropy = malloc(sizeof(mbedtls_entropy_context));

    mbedtls_ctr_drbg_init(crypto_p_rng);
    mbedtls_entropy_init(crypto_entropy);

    TRY(mbedtls_ctr_drbg_seed(crypto_p_rng,
                              mbedtls_entropy_func,
                              crypto_entropy,
                              (const unsigned char *)pers,
                              sizeof pers));

    crypto_aes_dec_ctx = calloc(sizeof(mbedtls_aes_context), 1);
    crypto_aes_enc_ctx = calloc(sizeof(mbedtls_aes_context), 1);
    mbedtls_aes_init(crypto_aes_dec_ctx);
    mbedtls_aes_init(crypto_aes_enc_ctx);
    return 0;

CATCH:
    crypto_free();
    return 1;
}

void crypto_free()
{

    mbedtls_aes_free(crypto_aes_dec_ctx);
    mbedtls_ecp_group_free(grp);
    mbedtls_ctr_drbg_free(crypto_p_rng);
    mbedtls_entropy_free(crypto_entropy);

    free(crypto_aes_dec_ctx);
    free(grp);
    free(crypto_p_rng);
    free(crypto_entropy);
    ok("** CRYPTO FREED **");
}