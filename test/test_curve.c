#include <stdio.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

#include "test.h"
#define DUMP_MPI(var) dump_mpi(&var, #var)
#define DUMP_POINT(var) dump_point(&var, #var)

mbedtls_ecp_group grp;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_entropy_context entropy;
mbedtls_ecp_point qA, qB;
mbedtls_mpi dA, dB, zA, zB;
void dump_mpi(mbedtls_mpi *mpi, const char *name)
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

void dump_point(mbedtls_ecp_point *P, const char *name)
{
    unsigned char buf[1024];
    size_t size = 0;
    mbedtls_ecp_point_write_binary(&grp, P, MBEDTLS_ECP_PF_UNCOMPRESSED,
                                   &size, buf, 1024);

    info("%s (%lu bytes):", name, size);
    hexdump(buf, size);
}
int test_main()
{

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
    DUMP_MPI(dA);
    DUMP_MPI(dB);
    DUMP_MPI(zA);
    DUMP_MPI(zB);
    DUMP_POINT(qA);
    DUMP_POINT(qB);
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
