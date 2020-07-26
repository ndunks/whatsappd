#include <fcntl.h>
#include "cfg.h"
#include "crypto.h"
#include "helper.h"
#include "json.h"
#include "wss.h"
#include "ssl.h"
#include "wasocket.h"
#include "session.h"

Me session_me;

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

static int session_handle_conn()
{
    size_t len;
    int i;
    char *buf,
        server_secret[CFG_SERVER_SECRET_LEN],
        *tokens[] = {
            "serverToken",
            "browserToken",
            "clientToken",
            0};
    info("session_handle_conn");
    // TRY(strncmp(buf, "[\"Conn\",{", 9));
    // TRY(json_parse_object(&buf));
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
    else
        accent("No secret in Conn.");

    if ((buf = json_get("serverToken")) != NULL)
    {
        strcpy(cfg->tokens.server, buf);
        accent("serverToken (%d):\n%s", strlen(buf), buf);
    }
    else
    {
        err("No serverToken");
        return 1;
    }

    if ((buf = json_get("browserToken")) != NULL)
    {
        strcpy(cfg->tokens.browser, buf);
        accent("browserToken (%d):\n%s", strlen(buf), buf);
    }
    else
    {
        err("No browserToken");
        return 1;
    }

    if ((buf = json_get("clientToken")) != NULL)
    {
        strcpy(cfg->tokens.client, buf);
        accent("clientToken (%d):\n%s", strlen(buf), buf);
    }
    else
    {
        err("No clientToken");
        return 1;
    }

    if ((buf = json_get("pushname")) != NULL)
    {
        strcpy(session_me.pushname, buf);
        accent("pushname (%d):\n%s", strlen(buf), buf);
    }
    else
        warn("No pushname");

    if ((buf = json_get("wid")) != NULL)
    {
        strcpy(session_me.wid, buf);
        accent("wid (%d):\n%s", strlen(buf), buf);
    }
    else
        warn("No wid");

    if ((buf = json_get("platform")) != NULL)
    {
        strcpy(session_me.platform, buf);
        accent("platform (%d):\n%s", strlen(buf), buf);
    }
    else
        warn("No platform");

    CATCH_RET = 0;
CATCH:
    return CATCH_RET;
}

static int session_handle_challenge(char *b64_client_id)
{
    char buf[1024] = {0}, challenge_b64[256],
         challenge_signed[256], challenge[256], *ptr, *msg, *msg_tag;

    ssize_t msg_size;
    size_t len;
    int status = 0;

    memset(challenge_signed, 0, 256);

    ptr = json_get("challenge");

    TRY(ptr == NULL);

    len = crypto_base64_decode(challenge, 256, ptr, strlen(ptr));
    TRY(len <= 0);

    accent("Challenge: %lu bytes", len);
    hexdump(challenge, len);
    accent("--------------");

    TRY(crypto_sign(challenge_signed, challenge, len))
    accent("Signed   : %lu bytes", len);
    hexdump(challenge_signed, len + 20);
    accent("--------------");

    crypto_base64_encode(challenge_b64, 256, challenge_signed, len);

    len = sprintf(buf, "[\"admin\",\"challenge\",\"%s\",\"%s\",\"%s\"]",
                  challenge_b64, cfg->tokens.server, b64_client_id);

    wasocket_send_text(buf, len, NULL);
    if (wasocket_read(&msg, &msg_tag, &msg_size))
        return 1;

    json_parse_object(&msg);

    if (!json_has("status"))
    {
        err("No status in Chalenge reply");
        return 1;
    }

    status = atoi(json_get("status"));
    accent("Chalenge reply: %d", status);
    CATCH_RET = status != 200;
CATCH:

    return CATCH_RET;
}

static int session_login_takeover()
{
    char buf[1024], b64_client_id[64], *msg, *msg_tag, *msg_prefix;
    ssize_t msg_size;
    int len, status;

    TRY(crypto_parse_server_keys(cfg->serverSecret, cfg));

    //init
    TRY(session_send_init(&b64_client_id));

    if (wasocket_read(&msg, &msg_tag, &msg_size))
        return 1;

    TRY(json_parse_object(&msg));
    TRY(strncmp("200", json_get("status"), 3));

    // take over
    len = sprintf(buf, "[\"admin\",\"login\",\"%s\",\"%s\",\"%s\",\"takeover\"]",
                  cfg->tokens.client, cfg->tokens.server, b64_client_id);

    TRY(wasocket_send_text(buf, len, NULL) < len);
    if (wasocket_read(&msg, &msg_tag, &msg_size))
        return 1;

    msg_prefix = msg;
    json_parse_object(&msg);

    if (strncmp(msg_prefix, "[\"Conn\",{", 9) == 0)
    {
        return session_handle_conn();
    }

    if (!json_has("type"))
    {
        err("session_login_takeover: No type in response");
        return 1;
    }

    if (strcmp(json_get("type"), "challenge") != 0)
    {
        err("No cannot handle type: %s", json_get("type"));
        return 1;
    }

    TRY(session_handle_challenge(b64_client_id));
    if (wasocket_read(&msg, &msg_tag, &msg_size))
        return 1;

    json_parse_object(&msg);
    TRY(session_handle_conn());

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
            if (wasocket_read(&msg, &msg_tag, &msg_size))
                return 1;

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

    if (wasocket_read(&msg, &msg_tag, &msg_size))
        return 1;

    TRY(json_parse_object(&msg));
    TRY(session_handle_conn());

    CATCH_RET = 0;

CATCH:

    return CATCH_RET;
}

// TODO: return the keys
int session_init(CFG *cfg_in)
{

    cfg = cfg_in;
    memset(&session_me, 0, sizeof(Me));

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
