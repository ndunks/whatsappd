#include "crypto.h"
#include "helper.h"
#include "wss.h"
#include "wasocket.h"
#include "session.h"

static CFG *cfg = NULL;

static int session_send_init()
{
    char buf[255], clientId[64], *reply;
    size_t size;

    crypto_base64_encode(clientId, 64, cfg->client_id, CFG_CLIENT_ID_LEN);

    size = sprintf(buf, "[\"admin\",\"init\","
                        "[2,2019,6],[\"whatsappd\",\"github.com/ndunks\",\"%s\"],"
                        "\"%s\",true]",
                   (sizeof(void *) == 4) ? "x86" : "x86_64",
                   clientId);

    if (wss_send_text(buf, size) != size)
    {
        err("Fail send init packed");
        return 1;
    };

    reply = wss_read(&size);
    info("repl: %s", reply);
    return 0;
}

int session_login_new()
{
    crypto_keys *keys = crypto_gen_keys();

    if (keys == NULL)
    {
        err("crypto_gen_keys fail");
        return 1;
    }

    TRY(crypto_random(cfg->client_id, CFG_CLIENT_ID_LEN));
    TRY(session_send_init(cfg));

CATCH:
    return CATCH_RET;
}

int session_start(CFG *cfg_in)
{
    cfg = cfg_in;

    TRY(wss_connect(NULL, NULL, NULL));
    wasocket_setup();

    if (cfg_has_credentials(cfg))
    {
        // login takeover
        CATCH_RET = 1;
        err("Unimplemented");
    }
    else
    {
        // new Login
        TRY(session_login_new(cfg));
    }
CATCH:
    return CATCH_RET;
}
