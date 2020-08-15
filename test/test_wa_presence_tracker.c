#include "whatsappd.h"
#include "test.h"
//https://web.whatsapp.com/send/?phone=6285726501011&text=urlencodedtext&source&data&app_absent

// int test_interactive()
// {
//     char number[25];
//     uint64_t jid_num;

// AGAIN:
//     printf("Number to check: ");
//     fflush(stdout);
//     scanf("%24s", number);
//     jid_num = helper_jid_to_num((void *)number);
//     if (!jid_num)
//     {
//         warn("Invalid number");
//         goto AGAIN;
//     }
//     info("Checking: %lu", jid_num);
//     ZERO(wa_action_presence_check(number));
//     goto AGAIN;
//     return 0;
// }

int test_once()
{
    char number[] = "6285726501111";
    WA_PRESENCE_CHECK_RESULT result;
    memset(&result, 0, sizeof(WA_PRESENCE_CHECK_RESULT));
    ZERO(wa_action_presence(1));
    wasocket_read_all(500);
    TRUTHY(wa_query_exist(number));
    wasocket_read_all(500);
    ok("%s is using WA", number);

    wa_query_profile_pic_thumb(number);

    ZERO(wa_action_presence_check(number, &result));

    info("type: %s, t: %ld, deny: %d", result.type, result.time, result.deny);
    return 0;
}

int test_main()
{
    ZERO(test_once());
    return 0;
}

int test_setup()
{

#ifdef HEADLESS
    err("This test require working .cfg");
    return TEST_SKIP;
#endif
    whatsappd_init(NULL);
    return 0;
}

int test_cleanup()
{
    whatsappd_free();
    return 0;
}
