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

static int session_handle_conn(char *data)
{
    ssize_t msg_size;
    size_t len;
    int i;
    char *msg, *msg_tag, server_secret[CFG_SERVER_SECRET_LEN];

    TRY(wasocket_read(&msg, &msg_tag, &msg_size));

    TRY(strncmp(msg, "[\"Conn\",{", 9));
    TRY(json_parse_object(&msg));
    i = json_find("secret");
    if (i >= 0)
    {
        len = strlen(json[i].value);
        ok("\nhas secret (%lu bytes)\n%s", len, json[i].value);
        if (crypto_base64_decode(
                server_secret,
                CFG_SERVER_SECRET_LEN,
                json[i].value,
                len) != CFG_SERVER_SECRET_LEN)
        {
            err("server_secret: Failed decoding base64");
            CATCH_RET = 1;
            goto CATCH;
        }
        info("server_secret:");
        hexdump(server_secret, CFG_SERVER_SECRET_LEN);
        info("-------------");
        TRY(crypto_parse_server_keys(server_secret, cfg));
    }

    if (json_has("serverToken"))
        strcpy(cfg->tokens.server, json_get("serverToken"));
    else
        warn("No serverToken");

    if (json_has("browserToken"))
        strcpy(cfg->tokens.browser, json_get("browserToken"));
    else
        warn("No browserToken");

    if (json_has("clientToken"))
        strcpy(cfg->tokens.client, json_get("clientToken"));
    else
        warn("No clientToken");

    TRY(wasocket_read(&msg, &msg_tag, &msg_size));
    printf("MSG: %s\n", msg);

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
    int reply_status = 0, seconds = 0, ttl = 20;
    ssize_t reply_size;
    char qrcode_content[1024] = {0}, b64_public_key[128],
         b64_client_id[64], *re_ref = "[\"admin\",\"Conn\",\"reref\"]",
         *reply, *reply_tag;

    cfg->cfg_file_version = 1;

    crypto_random(cfg->client_id, CFG_CLIENT_ID_LEN);
    crypto_keys_store_cfg(keys, cfg);
    crypto_base64_encode(b64_public_key, 128, cfg->keys.public, CFG_KEY_LEN);
    crypto_base64_encode(b64_client_id, 64, cfg->client_id, CFG_CLIENT_ID_LEN);
    crypto_keys_free(keys);

    TRY(session_send_init(b64_client_id));

    while (seconds < 120)
    {
        if (seconds == 0)
        {
            // reading reply
            TRY(wasocket_read(&reply, &reply_tag, &reply_size));
            TRY(json_parse_object(&reply));

            if (json_has("status"))
                reply_status = atoi(json_get("status"));
            else
                reply_status = 0;

            if (json_has("ttl"))
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
            }
        }

        CATCH_RET = ssl_check_read(1000);

        if (CATCH_RET < 0)
        {
            err("Socket error %x", CATCH_RET);
            goto CATCH;
        }

        if (CATCH_RET)
        {
            // Yes it have reply
            CATCH_RET = 0;
            break;
        }

        seconds++;
        printf("%d ", seconds);

        if (seconds >= ttl)
        {
            printf(COL_MAG "Refreshing QR Code...\n" COL_NORM);
            TRY(wasocket_send_text(re_ref, strlen(re_ref), NULL) <= 0);
            seconds = 0;
        }
    }

    TRY(session_handle_conn(reply));

    CATCH_RET = 0;

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
