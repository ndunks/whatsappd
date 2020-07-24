#include <fcntl.h>
#include "cfg.h"
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
    uint size;

    crypto_base64_encode(b64_client_id, 64, cfg->client_id, CFG_CLIENT_ID_LEN);

    size = sprintf(buf, "[\"admin\",\"init\","
                        "[2,2029,4],[\"whatsappd\",\"github.com/ndunks\",\"%s\"],"
                        "\"%s\",true]",
                   (sizeof(void *) == 4) ? "x86" : "x86_64",
                   b64_client_id);

    return wasocket_send_text(buf, size, NULL) < size;
}

static int session_handle_conn(char *data)
{
    ssize_t size;
    size_t len;
    int i;
    char *buf, *tag, server_secret[CFG_SERVER_SECRET_LEN],
        *tokens[] = {
            "serverToken",
            "browserToken",
            "clientToken",
            0};

    TRY(wasocket_read(&buf, &tag, &size));

    TRY(strncmp(buf, "[\"Conn\",{", 9));
    TRY(json_parse_object(&buf));
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
            return 1;
        }

        TRY(crypto_parse_server_keys(server_secret, cfg));
    }

    if ((buf = json_get("serverToken")) != NULL)
    {
        strcpy(cfg->tokens.server, buf);
        accent("serverToken (%d):\n%s", strlen(buf), buf);
    }
    else
        warn("No serverToken");

    if ((buf = json_get("browserToken")) != NULL)
    {
        strcpy(cfg->tokens.browser, buf);
        accent("browserToken (%d):\n%s", strlen(buf), buf);
    }
    else
        warn("No browserToken");

    if ((buf = json_get("clientToken")) != NULL)
    {
        strcpy(cfg->tokens.client, buf);
        accent("clientToken (%d):\n%s", strlen(buf), buf);
    }
    else
        warn("No clientToken");

    CATCH_RET = 0;
CATCH:
    return CATCH_RET;
}

static int session_login_takeover()
{
    char buf[1024], b64_client_id[64], *msg, *msg_tag;
    ssize_t msg_size;
    int len, status;

    //init
    TRY(session_send_init(&b64_client_id));
    TRY(wasocket_read(&msg, &msg_tag, &msg_size));
    TRY(json_parse_object(&msg));
    TRY(strncmp("200", json_get("status"), 3));

    // take over
    len = sprintf(buf, "[\"admin\",\"login\",\"%s\",\"%s\",\"%s\",\"takeover\"]",
                  cfg->tokens.client, cfg->tokens.server, b64_client_id);
    TRY(wasocket_send_text(buf, len, NULL) < len);
    TRY(wasocket_read(&msg, &msg_tag, &msg_size));
    json_parse_object(&msg);

    if (!json_has("status"))
    {
        if (strcmp(json_get("type"), "challenge") == 0)
        {
            err("Unimplemented handle chalenge");
            return 1;
        }
        else
        {
            err("No status reply in json");
            return 1;
        }
    }

    status = atoi(json_get("status"));
    len = 0;

    switch (status)
    {
    case 200:
        TRY(session_handle_conn(msg));
        return 0;

    case 401:
        len = sprintf(buf, "%s Unpaired from the phone", json_get("status"));
    case 403:
        if (len == 0)
            len = sprintf(buf, "%s Access denied", json_get("status"));

        len += sprintf(buf + len, ", remove config file %s and login again.", cfg_file_get());

        if (json_has("tos"))
        {
            len += sprintf(buf + len, "TOS: %s", json_get("tos"));
        }
        err("%s", buf);
        return 1;

    case 405:
        err("Already logged in");
        return 1;

    case 409:
        err("Logged in from another location");
        return 1;

    default:

        err("Unhandled status code: %s", json_get("status"));
        return 1;
    }

    CATCH_RET = 0;
CATCH:

    return CATCH_RET;
}

static int session_login_new()
{
    crypto_keys *keys = crypto_gen_keys();
    int msg_status = 0, seconds = 0, ttl = 20;
    ssize_t msg_size;
    char qrcode_content[1024] = {0}, b64_public_key[128],
         *re_ref = "[\"admin\",\"Conn\",\"reref\"]",
         b64_client_id[64], *msg, *msg_tag;

    cfg->cfg_file_version = 1;

    crypto_random(cfg->client_id, CFG_CLIENT_ID_LEN);
    crypto_keys_store_cfg(keys, cfg);
    crypto_base64_encode(b64_public_key, 128, cfg->keys.public, CFG_KEY_LEN);
    crypto_keys_free(keys);

    TRY(session_send_init(&b64_client_id));

    while (seconds < 120)
    {
        if (seconds == 0)
        {
            // reading msg
            TRY(wasocket_read(&msg, &msg_tag, &msg_size));
            TRY(json_parse_object(&msg));

            if (json_has("status"))
                msg_status = atoi(json_get("status"));
            else
                msg_status = 0;

            if (json_has("ttl"))
                ttl = atoi(json_get("ttl")) / 1000;

            info("QR CODE: status: %d, ttl: %d", msg_status, ttl);

            switch (msg_status)
            {
            case 200:
                sprintf(qrcode_content, "%s,%s,%s", json_get("ref"), b64_public_key, b64_client_id);
                helper_qrcode_show(qrcode_content);
                break;

            case 304:
                warn(" refresh QR CODE");
                seconds = ttl - 5; // try again after ~5secs
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

        if (CATCH_RET) // Yes it have reply
            break;

        if (++seconds >= ttl) // Refreshing QR Code
        {
            TRY(wasocket_send_text(re_ref, strlen(re_ref), NULL) <= 0);
            seconds = 0;
        }
    }

    TRY(session_handle_conn(msg));

    CATCH_RET = 0;

CATCH:

    return CATCH_RET;
}

// TODO: return the keys
int session_init(CFG *cfg_in)
{

    cfg = cfg_in;

    if (wss_connect(NULL, NULL, NULL) != 0)
    {
        err("Fail connecting to server.");
        return 1;
    }

    wasocket_setup();

    if (cfg_has_credentials(cfg))
        return session_login_takeover();
    else
        return session_login_new();
}

void session_free()
{
    wss_disconnect();
}
