#include "whatsappd.h"

/**
 * Login or resume session
 */
int whatsappd_init(const char const *config_path)
{
    int cfg_status;
    CFG cfg;

    memset(&cfg, 0, sizeof(CFG));

    cfg_status = cfg_file(config_path);
    if (cfg_status < 0)
    {
        err("Cannot Read/Write %s", cfg_file_get());
        return 1;
    }
    if (cfg_status == 1)
        TRY(cfg_load(&cfg));

    TRY(crypto_init());

    TRY(session_init(&cfg));

    cfg_save(&cfg);

    TRY(handler_preempt());

    CATCH_RET = 0;

CATCH:
    return CATCH_RET;
}

void whatsappd_free()
{
    session_free();
    crypto_free();
}

#ifndef TEST

int main(int argc, char const *argv[])
{
    // todo: arg parser
    TRY(whatsappd_init(NULL))

    //TRY(wss_connect());

CATCH:
    whatsappd_free();
    return CATCH_RET;
}

#endif