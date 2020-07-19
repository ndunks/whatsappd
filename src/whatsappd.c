#include <string.h>
#include <helper.h>
#include "wasocket.h"
#include "whatsappd.h"

/**
 * Login or resume session
 */
int whatsappd_init(const char const *config_path)
{
    CFG cfg;

    memset(&cfg, 0, sizeof(CFG));

    CATCH_RET = cfg_file(config_path);
    if (CATCH_RET < 0)
    {
        err("Cannot Read/Write %s", cfg_file_get());
        goto CATCH;
    }

    TRY(crypto_init());

    if (CATCH_RET == 1)
    {
        TRY(cfg_load(&cfg));
        CATCH_RET = crypto_parse_server_keys(cfg.serverSecret, &cfg);
        if (CATCH_RET)
        {
            err("Fail while parsing config: %s", config_path);
            goto CATCH;
        }
    }
    TRY(wasocket_init(&cfg));

    CATCH_RET = 0;

CATCH:
    return CATCH_RET;
}

void whatsappd_free()
{
    wasocket_free();
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