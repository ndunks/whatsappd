#include "session.h"
#include "crypto.h"
#include "helper.h"
#include "wasocket.h"

static int session_init(CFG *cfg)
{
    char buf[255], clientId[64];
    size_t size;

    crypto_base64_encode(clientId, 64, cfg->client_id, CFG_CLIENT_ID_LEN);

    size = sprintf(buf, "[\"admin\",\"init\","
                        "[2,2019,6],[\"whatsappd\",\"github.com/ndunks\",\"%s\"],"
                        "\"%s\",true]",
                   (sizeof(void *) == 4) ? "x86" : "x86_64",
                   clientId);
    if (ssl_write(buf, size) != size)
    {
        err("Fail send init packed");
        return 1;
    };
    return 0;
}

int session_new(CFG *cfg)
{
    crypto_keys *keys = crypto_gen_keys();

    if (keys == NULL)
    {
        err("crypto_gen_keys fail");
        return 1;
    }

    TRY(crypto_random(cfg->client_id, CFG_CLIENT_ID_LEN));
    TRY(session_init(cfg));

CATCH:

    crypto_free(keys);

    return CATCH_RET;
}