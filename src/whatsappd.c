#include <time.h>
#include "whatsappd.h"

// 20 bytes msg id + 1 for NULL. not support image id 16 bytes
void whatsappd_create_msg_id(const char *str)
{
    char randBytes[10];
    memset((void *)str, 0, 21);
    crypto_random(randBytes, 10);
    helper_buf_to_hex((uint8_t *)str, (uint8_t *)randBytes, 10);
}

void whatsappd_sanitize_jid(const char *dst, const char *number)
{
    char *at;
    strcpy(dst, number);
    at = strchr(dst, '@');

    if (at == NULL)
        strcat(dst, "@");
    else
    {
        if (strncmp(at + 1, wa_host_long, 14) == 0)
            return;

        *(at + 1) = 0;
    }

    strcat(dst, wa_host_long);
}

int whatsappd_send_text(const char *number, const char *text)
{
    MessageKey key;
    Message msg;
    WebMessageInfo info;
    size_t text_len = strlen(text);
    char *bin_buf = malloc(text_len + 256);

    key.fromMe = 1;
    whatsappd_create_msg_id(key.id);
    whatsappd_sanitize_jid(key.remoteJid, number);

    msg.conversation = text;

    info.key = &key;
    info.message = &msg;
    info.status = WEB_MESSAGE_INFO_STATUS_PENDING;
    info.messageTimestamp = time(NULL);

    // todo: binary writer
    buf_set(bin_buf, text_len + 256);
    proto_write_WebMessageInfo( &info );
    //crypto_decrypt_hmac();
    free(bin_buf);
}

/**
 * Login or resume session
 */
int whatsappd_init(const char *config_path)
{
    int cfg_status;
    CFG cfg;

    memset(&cfg, 0, sizeof(CFG));

    cfg_status = cfg_file(config_path);
    if (cfg_status < 0)
    {
        err("Cannot Read/Write %s", cfg_file_get());
        return 1;
    }
    if (cfg_status == 1)
        TRY(cfg_load(&cfg));

    TRY(crypto_init());

    TRY(session_init(&cfg));

    cfg_save(&cfg);

    TRY(handler_preempt());

    CATCH_RET = 0;

CATCH:
    return CATCH_RET;
}

void whatsappd_free()
{
    session_free();
    crypto_free();
}

#ifndef TEST

int main(int argc, char const *argv[])
{
    // todo: arg parser
    TRY(whatsappd_init(NULL))

    //TRY(wss_connect());

CATCH:
    whatsappd_free();
    return CATCH_RET;
}

#endif