#include "whatsappd.h"

/**
 * Should handle untill account logged in
 */
int whatsappd_init(const char const *config_path)
{
    CFG cfg;
    int ret;

    memset(&cfg, 0, sizeof(CFG));

    ret = cfg_file(config_path);
    if (ret < 0)
    {
        err("Cannot Read/Write %s", cfg_file_get());
        goto CATCH;
    }

    TRY(crypto_init());

    if (ret == 1)
    {
        TRY(cfg_load(&cfg));
        ret = crypto_parse_server_keys(cfg.serverSecret, &cfg);
        if (ret)
        {
            err("Fail while parsing config: %s", config_path);
            goto CATCH;
        }
    }

    TRY(wasocket_connect(NULL, NULL, NULL));

    if (cfg_has_credentials(&cfg))
    {
        // login takeover
        ret = 1;
        err("Unimplemented");
    }
    else
    {
        // new Login
        TRY(session_new(&cfg));
    }

    ret = 0;

CATCH:
    return ret;
}

void whatsappd_free()
{
    wasocket_disconnect();
    crypto_free();
}

#ifndef TEST

int main(int argc, char const *argv[])
{
    // todo: arg parser
    TRY(whatsappd_init(NULL))

    //TRY(wasocket_connect());

CATCH:
    whatsappd_free();
    return CATCH_RET;
}

#endif