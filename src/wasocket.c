#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include <helper.h>
#include <cfg.h>
#include <session.h>
#include <wss.h>

#include "wasocket.h"

/**
 * Should handle untill account logged in
 */
int wasocket_init(CFG *cfg)
{
    TRY(wss_connect(NULL, NULL, NULL));

    if (cfg_has_credentials(cfg))
    {
        // login takeover
        CATCH_RET = 1;
        err("Unimplemented");
    }
    else
    {
        // new Login
        TRY(session_new(cfg));
    }
CATCH:

    return CATCH_RET;
}

void wasocket_free()
{
    wss_disconnect();
}