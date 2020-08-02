#include <sys/file.h>
#include <unistd.h>

#include <cfg.h>
#include <crypto.h>
#include <binary_reader.h>
#include <helper.h>

#include <mbedtls/md.h>
#include <mbedtls/hkdf.h>
#include <mbedtls/sha256.h>
#include <mbedtls/aes.h>

#include "test.h"

u_char _whatsappd_cfg[] = {
    0x01, 0xa2, 0x2c, 0x50, 0x09, 0x4e, 0x9e, 0x06, 0x9b, 0xdd, 0x5e, 0x10,
    0xb8, 0x15, 0x63, 0x06, 0x30, 0x00, 0xf0, 0xc5, 0xaa, 0x15, 0xcb, 0xcb,
    0x49, 0xb7, 0x86, 0xb5, 0x74, 0xce, 0x86, 0xd0, 0x60, 0x6f, 0xad, 0x96,
    0x88, 0xb9, 0xb8, 0x37, 0x37, 0x0b, 0xf8, 0x49, 0x97, 0x2a, 0xf4, 0xf1,
    0x5a, 0x85, 0xac, 0x2f, 0xc8, 0x9c, 0x8e, 0x16, 0x4d, 0x0f, 0x9b, 0x41,
    0xb6, 0xca, 0x5a, 0xf7, 0x48, 0x9f, 0x79, 0x78, 0x8d, 0x29, 0x68, 0xd7,
    0x94, 0x7c, 0xe7, 0x50, 0x17, 0x64, 0xbf, 0xc9, 0x37, 0x6f, 0x6a, 0x63,
    0x64, 0x63, 0x42, 0x6c, 0x63, 0x50, 0x77, 0x32, 0x46, 0x42, 0x4e, 0x44,
    0x36, 0x5a, 0x69, 0x2b, 0x76, 0x44, 0x37, 0x4d, 0x35, 0x2b, 0x62, 0x58,
    0x33, 0x58, 0x6b, 0x62, 0x4b, 0x42, 0x50, 0x57, 0x33, 0x69, 0x37, 0x72,
    0x72, 0x70, 0x4f, 0x6f, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x31, 0x40, 0x72, 0x67, 0x56, 0x56, 0x74, 0x39, 0x37, 0x74, 0x46,
    0x77, 0x53, 0x70, 0x50, 0x33, 0x69, 0x57, 0x63, 0x5a, 0x54, 0x34, 0x37,
    0x47, 0x6e, 0x63, 0x41, 0x4e, 0x58, 0x78, 0x5a, 0x4d, 0x6b, 0x34, 0x2b,
    0x39, 0x39, 0x4e, 0x75, 0x50, 0x67, 0x71, 0x68, 0x36, 0x33, 0x31, 0x53,
    0x48, 0x4d, 0x32, 0x43, 0x35, 0x4e, 0x79, 0x6a, 0x44, 0x72, 0x69, 0x53,
    0x53, 0x43, 0x66, 0x6b, 0x39, 0x4d, 0x47, 0x4d, 0x34, 0x62, 0x54, 0x58,
    0x6a, 0x59, 0x69, 0x4b, 0x51, 0x4f, 0x70, 0x49, 0x51, 0x3d, 0x3d, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x40, 0x6e, 0x53, 0x65, 0x67, 0x79,
    0x56, 0x73, 0x36, 0x71, 0x6c, 0x46, 0x71, 0x50, 0x6b, 0x62, 0x61, 0x64,
    0x51, 0x63, 0x48, 0x75, 0x43, 0x65, 0x66, 0x44, 0x69, 0x69, 0x6f, 0x4c,
    0x6d, 0x67, 0x46, 0x37, 0x65, 0x4b, 0x61, 0x6c, 0x4a, 0x79, 0x4a, 0x39,
    0x4b, 0x4c, 0x5a, 0x6e, 0x52, 0x69, 0x4f, 0x55, 0x38, 0x61, 0x33, 0x6e,
    0x68, 0x53, 0x72, 0x2b, 0x50, 0x41, 0x4b, 0x50, 0x6e, 0x4f, 0x47, 0x34,
    0x77, 0x57, 0x34, 0x6d, 0x4a, 0x32, 0x51, 0x4a, 0x45, 0x36, 0x49, 0x6a,
    0x79, 0x6c, 0x4e, 0x48, 0x35, 0x5a, 0x71, 0x63, 0x52, 0x42, 0x2b, 0x39,
    0x75, 0x4c, 0x48, 0x6b, 0x75, 0x38, 0x31, 0x77, 0x50, 0x31, 0x2f, 0x58,
    0x6f, 0x4e, 0x34, 0x54, 0x6e, 0x57, 0x4f, 0x69, 0x4d, 0x45, 0x37, 0x35,
    0x2f, 0x61, 0x55, 0x6a, 0x63, 0x4b, 0x4c, 0x68, 0x6f, 0x47, 0x31, 0x30,
    0x41, 0x67, 0x36, 0x4d, 0x6a, 0x49, 0x34, 0x2b, 0x76, 0x6f, 0x32, 0x4c,
    0x75, 0x31, 0x45, 0x41, 0x4e, 0x68, 0x6d, 0x52, 0x58, 0x65, 0x5a, 0x42,
    0x67, 0x3d, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x1f, 0x4d,
    0xea, 0x5b, 0xc4, 0x94, 0xcf, 0xc5, 0x11, 0xbe, 0xc5, 0x3f, 0xff, 0xd2,
    0xde, 0x78, 0xea, 0xc9, 0xde, 0x69, 0xdc, 0x07, 0x3d, 0x5b, 0x6e, 0x65,
    0x88, 0xb2, 0xb5, 0x4a, 0x37, 0x48, 0x4f, 0x64, 0xc9, 0xb4, 0xf8, 0x44,
    0x5a, 0x27, 0x60, 0x73, 0x19, 0x1a, 0x88, 0x0a, 0x04, 0xd6, 0x90, 0x47,
    0x0b, 0xe0, 0x8b, 0x53, 0xc4, 0x50, 0x15, 0xf2, 0x4c, 0xb2, 0x26, 0x93,
    0x0d, 0x42, 0x6a, 0x7b, 0xf5, 0xbd, 0x93, 0xa0, 0x3a, 0x33, 0x50, 0x18,
    0xce, 0x35, 0x57, 0x81, 0xfe, 0x55, 0x3c, 0xa9, 0xd3, 0x98, 0x6e, 0xa2,
    0x20, 0xf5, 0x22, 0x59, 0xd0, 0x4e, 0x55, 0x5d, 0xb9, 0xf8, 0x97, 0x7c,
    0x16, 0x70, 0x3f, 0x3e, 0xf0, 0x41, 0xb4, 0x44, 0xbe, 0xa5, 0x76, 0xad,
    0x7c, 0x96, 0xd0, 0x34, 0xc0, 0x42, 0xa2, 0xa0, 0x72, 0xf8, 0x02, 0xf0,
    0x49, 0xb7, 0x58, 0x08, 0xf9, 0x63, 0x1f, 0x87, 0x91, 0x5d, 0x00, 0x59,
    0x63, 0x78, 0xfd, 0xa4, 0x37, 0x8e, 0xc1, 0x89, 0x18};
// preempt_1
u_char encrypted[] = {
    0x4f, 0x25, 0x40, 0xbf, 0x6f, 0x49, 0x10, 0x94, 0xc9, 0x26, 0x36, 0xeb,
    0x19, 0xf9, 0xdf, 0x01, 0x3c, 0x97, 0x95, 0x87, 0x7a, 0x13, 0x3e, 0x96,
    0x14, 0xd0, 0x91, 0xc7, 0x23, 0xa1, 0xee, 0xd1, 0x52, 0x2a, 0x0f, 0x3d,
    0xf7, 0xb8, 0x74, 0x67, 0x6d, 0xb5, 0x75, 0x42, 0xbc, 0x73, 0x13, 0x98,
    0xe4, 0x3a, 0xdc, 0x37, 0x08, 0x1e, 0x6b, 0x1a, 0x70, 0x3e, 0xf2, 0x5e,
    0xa2, 0x99, 0x23, 0x37, 0xcd, 0x34, 0x37, 0xc9, 0x24, 0xc8, 0xac, 0x04,
    0x47, 0xba, 0xbf, 0xac, 0x07, 0x3c, 0x14, 0xab, 0xca, 0x6a, 0xe1, 0x7d,
    0x4c, 0xc1, 0x69, 0x8e, 0x9a, 0x2b, 0x84, 0x80, 0xac, 0x7c, 0x94, 0x70,
    0x94, 0x35, 0x74, 0xe1, 0x33, 0x1f, 0x9d, 0xd7, 0xff, 0xe9, 0x0b, 0x47,
    0xba, 0x75, 0x50, 0x44, 0x9c, 0x6f, 0xee, 0x4a, 0xf7, 0xd5, 0xdd, 0xe9,
    0x8d, 0xaa, 0xf2, 0x7e, 0x53, 0x6c, 0xba, 0xe7};
u_char decrypted[] = {
    0xf8, 0x04, 0x4d, 0x5b, 0x13, 0xf8, 0x02, 0xf8, 0x0b, 0x13, 0x2d, 0xfa,
    0xff, 0x87, 0x62, 0x85, 0x72, 0x65, 0x01, 0x01, 0x7f, 0x50, 0x18, 0xff,
    0x81, 0x1f, 0x57, 0xff, 0x05, 0x15, 0x94, 0x10, 0x09, 0x89, 0x63, 0xff,
    0x81, 0x0f, 0x71, 0x20, 0xf8, 0x0b, 0x13, 0x2d, 0xfa, 0xff, 0x87, 0x62,
    0x82, 0x31, 0x34, 0x39, 0x39, 0x3f, 0x50, 0x18, 0xff, 0x81, 0x0f, 0x57,
    0xff, 0x05, 0x15, 0x94, 0x09, 0x71, 0x27, 0x63, 0xff, 0x81, 0x0f, 0x71,
    0x20};
size_t encrypted_size = 128;
size_t decrypted_size = 73;
int test_main()
{
    BINARY_NODE *node;
    int ret = 0;
    u_char work_buf[256] = {0};
    CFG *cfg = (void *)&_whatsappd_cfg[0];
    u_char *res = &encrypted[0];
    hexdump(cfg->serverSecret, 144);
    ZERO(crypto_parse_server_keys(cfg->serverSecret, cfg));

    memset(work_buf, 0, 256);
    //ret = crypto_decrypt_hmac(&res, &encrypted_size, work_buf);
    // ---------------
    mbedtls_aes_context aes_ctx;
    uint8_t hmac_check[32], *sign, *iv, *input;
    size_t check_len = 0, decrypted_len = 0;
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx, (uint8_t *)crypto_aes_keys.enc, 32 * 8);
    mbedtls_aes_setkey_enc(&aes_ctx, (uint8_t *)crypto_aes_keys.enc, 32 * 8);
    memset(hmac_check, 0, 32);

    sign = encrypted;
    iv = encrypted + 32;
    input = encrypted + 32 + 16;
    check_len = encrypted_size - 32; // hmac sign
    decrypted_len = check_len - 16;  // iv
    ZERO(mbedtls_md_hmac(
        mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
        (uint8_t *)crypto_aes_keys.mac,
        32,
        iv, // -> src + 32
        check_len,
        hmac_check));
    if (memcmp(sign, hmac_check, 32) != 0)
    {
        err("HMAC Not match!");
        return 1;
    }
    ZERO(mbedtls_aes_crypt_cbc(
        &aes_ctx,
        MBEDTLS_AES_DECRYPT,
        decrypted_len,
        iv,
        input,
        (uint8_t *)res));

    mbedtls_aes_free(&aes_ctx);
    // ---------------

    hexdump(res, encrypted_size);
    ZERO(memcmp(decrypted, res, decrypted_size));
    TRUTHY(decrypted_size == encrypted_size);
    node = binary_read(res, encrypted_size);

    ZERO(ret);
    return 0;
}

int test_setup()
{

    ZERO(crypto_init());
    return 0;
}

int test_cleanup()
{
    warn("test_cleanup");
    crypto_free();
    return 0;
}
