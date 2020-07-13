#include <stdio.h>
#include <crypto.h>
#include "test.h"

int test_main()
{
    mbedtls_ecp_group grp;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    mbedtls_ecp_point qA, qB;
    mbedtls_mpi dA, dB, zA, zB;
    const char pers[] = "ecdh";
    int ret;

    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&qA);
    mbedtls_ecp_point_init(&qB);
    mbedtls_mpi_init(&dA);
    mbedtls_mpi_init(&dB);
    mbedtls_mpi_init(&zA);
    mbedtls_mpi_init(&zB);

    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *)pers,
                                     sizeof pers)) != 0)
    {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto exit;
    }

    ZERO(mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_CURVE25519));

    ZERO(mbedtls_ecdh_gen_public(&grp, &dA, &qA,
                                 mbedtls_ctr_drbg_random,
                                 &ctr_drbg));
    ZERO(mbedtls_ecdh_gen_public(&grp, &dB, &qB,
                                 mbedtls_ctr_drbg_random,
                                 &ctr_drbg));


    ZERO(mbedtls_ecdh_compute_shared(&grp, &zA, &qB, &dA,
                                     mbedtls_ctr_drbg_random,
                                     &ctr_drbg));

    ZERO(mbedtls_ecdh_compute_shared(&grp, &zB, &qA, &dB,
                                     mbedtls_ctr_drbg_random,
                                     &ctr_drbg));
    CRYPTO_DUMP_MPI(dA);
    CRYPTO_DUMP_MPI(dB);
    CRYPTO_DUMP_MPI(zA);
    CRYPTO_DUMP_MPI(zB);
    CRYPTO_DUMP_POINT(qA);
    CRYPTO_DUMP_POINT(qB);
    ZERO(mbedtls_mpi_cmp_mpi(&zA, &zB));

exit:
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&qA);
    mbedtls_ecp_point_free(&qB);
    mbedtls_mpi_free(&dA);
    mbedtls_mpi_free(&dB);
    mbedtls_mpi_free(&zA);
    mbedtls_mpi_free(&zB);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    return 0;
}

int test_setup()
{
    return 0;
}

int test_cleanup()
{
    return 0;
}
