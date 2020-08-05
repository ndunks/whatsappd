#include "crypto.h"

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
