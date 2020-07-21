#include "crypto.h"
#include "helper.h"
#include "wss.h"
#include "wasocket.h"
#include "session.h"

static CFG *cfg = NULL;

static int session_send_init(struct HELPER_JSON_INIT_REPLY *response, char *b64_client_id)
{
    char buf[255], *data, *tag;
    union {
        size_t u;
        ssize_t s;
    } size;

    size.u = sprintf(buf, "[\"admin\",\"init\","
                          "[2,2029,4],[\"whatsappd\",\"github.com/ndunks\",\"%s\"],"
                          "\"%s\",true]",
                     (sizeof(void *) == 4) ? "x86" : "x86_64",
                     b64_client_id);

    if (wasocket_send_text(buf, size.u, NULL) < size.u)
    {
        err("Fail send init packed");
        return 1;
    };

    TRY(wasocket_read(&data, &tag, &size));
    TRY(size.s < 10);

    TRY(helper_parse_init_reply(response, data));

CATCH:
    return CATCH_RET;
}

static int session_login_takeover()
{
    return 0;
}

static int session_login_new()
{
    struct HELPER_JSON_INIT_REPLY response;
    crypto_keys *keys = crypto_gen_keys();
    int len;
    char qrcode_content[1024] = {0}, b64_public_key[128], b64_client_id[64];

    TRY(keys == NULL)

    TRY(crypto_random(cfg->client_id, CFG_CLIENT_ID_LEN));
    TRY(crypto_keys_store_cfg(keys, cfg));
    TRY(crypto_base64_encode(b64_public_key, 128, cfg->keys.public, CFG_KEY_LEN) <= 0);
    TRY(crypto_base64_encode(b64_client_id, 64, cfg->client_id, CFG_CLIENT_ID_LEN) <= 0);
    TRY(session_send_init(&response, b64_client_id));

    TRY(helper_json_unescape(&response.ref));

    TRY(strlen(response.ref) <= 10);

    len = sprintf(qrcode_content, "%s,%s,%s", response.ref, b64_public_key, b64_client_id);
    info("QR CODE CONTENT(%lu): %s", len, qrcode_content);

CATCH:
    crypto_free(keys);
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
