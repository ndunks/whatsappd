#pragma once
#include "util.h"
#include "binary.h"

typedef enum WIRETYPE
{
    WIRETYPE_VARINT = 0,
    WIRETYPE_FIXED64 = 1,
    WIRETYPE_LENGTH_DELIMITED = 2,
    WIRETYPE_START_GROUP = 3,
    WIRETYPE_END_GROUP = 4,
    WIRETYPE_FIXED32 = 5,
} WIRETYPE;

typedef struct PROTO
{
    uint32_t field;
    WIRETYPE type;
    size_t len;

    union value {
        char *buf;
        uint64_t num64;
        int64_t snum64;
        uint32_t num32;
        int32_t snum32;
    } value;

    char *ptr;
} PROTO;

typedef struct Message
{
    // optional string conversation = 1;
    char *conversation;
    // optional SenderKeyDistributionMessage senderKeyDistributionMessage = 2;
    // optional ImageMessage imageMessage = 3;
    // optional ContactMessage contactMessage = 4;
    // optional LocationMessage locationMessage = 5;
    // optional ExtendedTextMessage extendedTextMessage = 6;
    // optional DocumentMessage documentMessage = 7;
    // optional AudioMessage audioMessage = 8;
    // optional VideoMessage videoMessage = 9;
    // optional Call call = 10;
    // optional Chat chat = 11;
    // optional ProtocolMessage protocolMessage = 12;
    // optional ContactsArrayMessage contactsArrayMessage = 13;
    // optional HighlyStructuredMessage highlyStructuredMessage = 14;
    // optional SenderKeyDistributionMessage fastRatchetKeySenderKeyDistributionMessage = 15;
    // optional SendPaymentMessage sendPaymentMessage = 16;
    // optional LiveLocationMessage liveLocationMessage = 18;
    // optional RequestPaymentMessage requestPaymentMessage = 22;
    // optional DeclinePaymentRequestMessage declinePaymentRequestMessage = 23;
    // optional CancelPaymentRequestMessage cancelPaymentRequestMessage = 24;
    // optional TemplateMessage templateMessage = 25;
    // optional StickerMessage stickerMessage = 26;
    // optional GroupInviteMessage groupInviteMessage = 28;
    // optional TemplateButtonReplyMessage templateButtonReplyMessage = 29;
    // optional ProductMessage productMessage = 30;
    // optional DeviceSentMessage deviceSentMessage = 31;
} Message;

typedef struct MessageKey
{
    // optional string remoteJid = 1;
    char remoteJid[48];
    // optional bool fromMe = 2;
    bool fromMe;
    // optional string id = 3;
    char id[64];
    // optional string participant = 4;
    char participant[64];
} MessageKey;

typedef enum WEB_MESSAGE_INFO_STATUS
{
    WEB_MESSAGE_INFO_STATUS_ERROR = 0,
    WEB_MESSAGE_INFO_STATUS_PENDING = 1,
    WEB_MESSAGE_INFO_STATUS_SERVER_ACK = 2,
    WEB_MESSAGE_INFO_STATUS_DELIVERY_ACK = 3,
    WEB_MESSAGE_INFO_STATUS_READ = 4,
    WEB_MESSAGE_INFO_STATUS_PLAYED = 5,
} WEB_MESSAGE_INFO_STATUS;

typedef struct WebMessageInfo
{
    // required MessageKey key = 1
    MessageKey *key;
    // optional Message message = 2;
    Message *message;
    // optional uint64 messageTimestamp = 3
    uint32_t messageTimestamp;
    // Status only filled when message is fromMe = true
    // optional WEB_MESSAGE_INFO_STATUS status = 4
    WEB_MESSAGE_INFO_STATUS status;
} WebMessageInfo;

uint proto_varint_write(uint32_t num);
uint32_t proto_varint_read();
uint proto_varint_size(uint64_t num);

int proto_tag_read(PROTO *proto);
int proto_tag_write(PROTO *proto);
size_t proto_tag_size(uint32_t field);

PROTO *protos_get(uint32_t field, PROTO *proto, size_t max);
size_t proto_size(PROTO *proto, int proto_len);
int proto_write(PROTO *proto);
int proto_writes(PROTO *proto, int proto_len);
int proto_scan(PROTO *proto, int proto_len, int max_field);

int proto_parse_WebMessageInfo(WebMessageInfo *dst, char *buf, size_t buf_size);
int proto_write_WebMessageInfo(WebMessageInfo *src);
void proto_free_WebMessageInfo(WebMessageInfo *src);

int proto_parse_MessageKey(MessageKey *dst, char *buf, size_t buf_size);
int proto_write_MessageKey(MessageKey *src, PROTO *protos, int proto_len);
void proto_free_MessageKey(MessageKey *src);

int proto_parse_Message(Message *dst, char *buf, size_t buf_size);
int proto_write_Message(Message *src, PROTO *protos, int proto_len);
void proto_free_Message(Message *src);