#include <fcntl.h>
#include "crypto.h"
#include "helper.h"
#include "json.h"
#include "wss.h"
#include "ssl.h"
#include "wasocket.h"
#include "session.h"

static CFG *cfg = NULL;

static int session_send_init(char *b64_client_id)
{
    char buf[255];
    size_t size;

    size = sprintf(buf, "[\"admin\",\"init\","
                        "[2,2029,4],[\"whatsappd\",\"github.com/ndunks\",\"%s\"],"
                        "\"%s\",true]",
                   (sizeof(void *) == 4) ? "x86" : "x86_64",
                   b64_client_id);

    return wasocket_send_text(buf, size, NULL) < size;
}

static session_handle_conn(){
   
}
static int session_login_takeover()
{
    return 0;
}

static int session_login_new()
{
    crypto_keys *keys = crypto_gen_keys();
    int reply_status = 0, seconds = 0, ttl = 20;
    ssize_t reply_size;
    char qrcode_content[1024] = {0}, b64_public_key[128],
         b64_client_id[64], *re_ref = "[\"admin\",\"Conn\",\"reref\"]",
         *reply, *reply_tag;

    TRY(keys == NULL);

    TRY(crypto_random(cfg->client_id, CFG_CLIENT_ID_LEN));
    TRY(crypto_keys_store_cfg(keys, cfg));
    TRY(crypto_base64_encode(b64_public_key, 128, cfg->keys.public, CFG_KEY_LEN) <= 0);
    TRY(crypto_base64_encode(b64_client_id, 64, cfg->client_id, CFG_CLIENT_ID_LEN) <= 0);

    TRY(session_send_init(b64_client_id));

    while (seconds < 120)
    {
        if (seconds == 0)
        {
            // reading reply
            TRY(wasocket_read(&reply, &reply_tag, &reply_size));
            TRY(json_parse_object(&reply));

            if (json_has( "status" ))
                reply_status = atoi(json_get("status"));
            else
                reply_status = 0;

            if (json_has( "ttl" ))
                ttl = atoi(json_get("ttl")) / 1000;

            info("QR CODE: status: %d, ttl: %d", reply_status, ttl);

            switch (reply_status)
            {
            case 200:
        
                sprintf(qrcode_content, "%s,%s,%s", json_get("ref"), b64_public_key, b64_client_id);
                helper_qrcode_show(qrcode_content);
                break;

            case 304:
                warn("Not yet refresh QR CODE");
                // try again after ~5secs
                seconds = ttl - 5;
                break;
            case 429:
                err("Login timeout!");

            default:
                CATCH_RET = 1;
                goto CATCH;
                break;
            }
        }

        CATCH_RET = ssl_check_read(1000);
        
        if (CATCH_RET < 0)
        {
            err("Socket error %x", CATCH_RET);
            goto CATCH;
            break;
        }

        if (CATCH_RET)
        {
            // Yes it have reply
            CATCH_RET = 0;
            break;
        }

        seconds++;
        info("wait %d", seconds);

        if (seconds >= ttl)
        {
            printf(COL_MAG "Refreshing QR Code...\n" COL_NORM);
            TRY(wasocket_send_text(re_ref, strlen(re_ref), NULL) <= 0);
            seconds = 0;
        }
    }

    TRY(wasocket_read(&reply, &reply_tag, &reply_size));
    printf("MSG: %s", reply);
    TRY(wasocket_read(&reply, &reply_tag, &reply_size));
    printf("MSG: %s", reply);

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
