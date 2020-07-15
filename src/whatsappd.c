#include "whatsappd.h"

int whatsappd_init(const char const *config_path)
{
    CFG *cfg = NULL;
    int ret = cfg_file(config_path);

    if (ret < 0)
    {
        err("Cannot Read/Write %s", cfg_file_get());
        goto CATCH;
    }

    TRY(crypto_init());
    TRY(wasocket_connect());

    if (ret == 1)
    {
        // Load config & crypto
        cfg = calloc(sizeof(CFG), 1);
        TRY(cfg_load(cfg));
        ret = crypto_parse_server_keys(cfg->serverSecret, cfg);
        if (ret)
        {
            err("Fail while parsing config: %s", config_path);
            goto CATCH;
        }
    }
    else
    {
        // init new connection
    }

    ret = 0;

CATCH:

    if (cfg != NULL)
        free(cfg);

    return ret;
}

void whatsappd_free()
{
    crypto_free();
}

#ifndef TEST

int main(int argc, char const *argv[])
{
    // todo: arg parser
    if (whatsappd_init(NULL))
    {
        err("whatsappd_init Fail");
        return 1;
    }

    //TRY(wasocket_connect());

    return 0;

CATCH:
    whatsappd_free();
    return 1;
}

#endif