#include "crypto.h"
#include "helper.h"
#include "wss.h"
#include "wasocket.h"
#include "session.h"

static CFG *cfg = NULL;

static int session_send_init()
{
    char buf[255], clientId[64], *data, *tag;
    union {
        size_t u;
        ssize_t s;
    } size;

    crypto_base64_encode(clientId, 64, cfg->client_id, CFG_CLIENT_ID_LEN);

    size.u = sprintf(buf, "[\"admin\",\"init\","
                          "[2,2029,4],[\"whatsappd\",\"github.com/ndunks\",\"%s\"],"
                          "\"%s\",true]",
                     (sizeof(void *) == 4) ? "x86" : "x86_64",
                     clientId);

    if (wasocket_send_text(buf, size.u, NULL) < size.u)
    {
        err("Fail send init packed");
        return 1;
    };
    TRY(wasocket_read(&data, &tag, &size));
    info("repl: (%ld) %s", size.s, data);
    CATCH_RET = 0;

CATCH:
    return CATCH_RET;
}

static int session_login_takeover()
{
    return 0;
}

static int session_login_new()
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

int session_init(CFG *cfg_in)
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

    CATCH_RET = 0;
CATCH:
    return CATCH_RET;
}

void session_free()
{
    wss_disconnect();
}
