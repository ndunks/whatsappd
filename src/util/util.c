#include "util.h"

#ifdef SAVE_MSG

static char *util_save_msg_file[1024];
static uint util_save_msg_counter = 0;

void util_save_msg(char *tag, char *plain, size_t plain_len, char *encrypted, size_t encrypted_len, bool is_binary)
{
    util_save_msg_counter++;
    sprintf(util_save_msg_file, "%s/%02u_%s.%s", SAVE_MSG, util_save_msg_counter, tag, is_binary ? "bin" : "txt");
    
    helper_save_file(util_save_msg_file, plain, plain_len);

    if (encrypted != NULL)
    {
        strcat(util_save_msg_file, ".enc");
        helper_save_file(util_save_msg_file, encrypted, encrypted_len);
    }
}

#endif
