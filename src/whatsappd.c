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

void whatsappd_sanitize_jid(char *dst, const char *number)
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

int whatsappd_send_text(const char *number, const char *const text)
{
    MessageKey key;
    Message msg;
    WebMessageInfo info;
    size_t buf_len, node_size, text_len = strlen(text);
    char *bin_buf, *node_buf;
    BINARY_NODE node_action, node_message;
    BINARY_NODE_ATTR *attr;
    buf_len = text_len + 512;

    memset(&node_action, 0, sizeof(BINARY_NODE));
    memset(&node_message, 0, sizeof(BINARY_NODE));
    //node_message = childs;

    bin_buf = malloc(buf_len);
    node_buf = malloc(buf_len);

    key.fromMe = 1;
    whatsappd_create_msg_id(key.id);
    whatsappd_sanitize_jid(key.remoteJid, number);

    msg.conversation = text;

    info.key = &key;
    info.message = &msg;
    info.status = WEB_MESSAGE_INFO_STATUS_PENDING;
    info.messageTimestamp = time(NULL);

    buf_set(bin_buf, text_len + 256);
    proto_write_WebMessageInfo(&info);

    node_action.tag = "action";

    attr = &node_action.attrs[node_action.attr_len++];
    attr->key = "type";
    attr->value = "relay";

    attr = &node_action.attrs[node_action.attr_len++];
    attr->key = "epoch";
    attr->value = "1";

    node_message.tag = "message";
    node_message.child_type = BINARY_NODE_CHILD_BINARY;
    node_message.child_len = buf_idx;
    node_message.child.data = bin_buf;

    node_action.child_type = BINARY_NODE_CHILD_LIST;
    node_action.child_len = 1;
    node_action.child.list = malloc(sizeof(void *));
    node_action.child.list[0] = &node_message;
    
    node_size = binary_write(&node_action, node_buf, buf_len);
    info("node_size %lu", node_size);
    TRY(node_size == 0);
    memset(bin_buf, 0, buf_len);
    CHECK(crypto_encrypt_hmac(node_buf, node_size, bin_buf, &buf_len));
    info("crypted_size %lu", buf_len);

    buf_set(node_buf, buf_len);

    buf_write_byte(BINARY_METRIC_MESSAGE);
    // EphemeralFlag Ignore
    buf_write_byte(1U << 7);
    buf_write_bytes(node_buf, buf_len);
    wasocket_send_binary(bin_buf, buf_idx, key.id);

CATCH:
    //crypto_decrypt_hmac();
    free(bin_buf);
    free(node_buf);
    return CATCH_RET;
    return 0;
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