#include "lang.h"
#include "test.h"

int test_main()
{
    TRUTHY(lang_detect_by_num((uint64_t)6200000000000) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)6285726501017) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)628572650101) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)62857265010) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)6285726501) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)628572650) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)62857265) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)6285726) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)628572) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)62857) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)6285) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)628) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)62) == LANG_ID);
    TRUTHY(lang_detect_by_num((uint64_t)99) == LANG_EN);

    for (int i = 0; i < UINT8_MAX; i++)
    {
        if (i == 62)
            continue;
        TRUTHY(lang_detect_by_num((uint64_t)i) == LANG_EN);
    }

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
