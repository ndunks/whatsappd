#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
//#include <pthread.h>
#include <byteswap.h>
#include <helper.h>
#include <color.h>

#include "wss.h"

#define WSS_FRAGMENT_MAX 100
#define WSS_NEED(len, x, x_len, x_size)          \
    while ((x_len + len) > x_size)               \
    {                                            \
        x_size += len + 1;                       \
        x = realloc(x, x_size);                  \
        if (x == NULL)                           \
            die("wss: Fail realloc " #x);        \
        warn("wss: realloc " #x " %lu", x_size); \
    }

#define WSS_NEED_TX(len) WSS_NEED(len, wss.tx, wss.tx_len, wss.tx_size)
#define WSS_NEED_RX(len) WSS_NEED(len, wss.rx, wss.rx_len, wss.rx_size)

struct WSS
{
    char *tx, *rx;
    size_t rx_len, rx_size, rx_idx,
        tx_len, tx_size, tx_idx;
} wss;

struct PAYLOAD
{
    char *data;
    uint64_t size;
    uint8_t frame_size;
};

struct FRAME
{
    uint8_t fin,   //  1 bit
        rsv1,      //  1 bit
        rsv2,      //  1 bit
        rsv3,      //  1 bit
        opcode,    //  4 bits
        masked;    //  1 bit
    uint32_t mask; // 32 bits
    uint8_t payload_count;
    /* Payload data len only */
    size_t payload_size;
    struct PAYLOAD payloads[WSS_FRAGMENT_MAX];
} frame;

uint32_t wss_mask()
{
    uint32_t mask = 0;
    srand(time(0) + rand());
    for (int i = 0; i < 4 / sizeof(int); i++)
    {
        ((int *)&mask)[i] = rand();
    }
    return mask;
}

static void wss_write(uint8_t *data, size_t len, uint8_t mask[4])
{
    size_t i;

    WSS_NEED_TX(len);

    for (i = 0; i < len; i++)
    {
        wss.tx[wss.tx_len + i] = (data[i] ^ mask[i % 4]);
    }
    wss.tx_len += len;
}

static void wss_write_int(uint8_t *data, uint8_t len)
{
    WSS_NEED_TX(len);

    for (int i = 0; i < len; i++)
    {
        //reversed
        wss.tx[wss.tx_len++] = data[len - (1 + i)];
    }
}

/* Read int big-endian to little */
/* static void wss_read_int(uint8_t *dst, size_t offset, uint8_t len)
{
    for (int i = 0; i < len; i++)
        dst[len - (1 + i)] = wss.rx[offset + i];
} */

static int wss_send()
{
    size_t sent, total = 0;
    if (wss.tx_len < 255)
    {
        hexdump(wss.tx, wss.tx_len);
    }

    do
    {
        sent = ssl_write(&wss.tx[total], wss.tx_len - total);
        total += sent;
    } while (total < wss.tx_len && sent > 0);
    warn(">> %lu bytes", sent);
    return total;
}

// https://tools.ietf.org/html/rfc6455#page-31
static size_t wss_frame(enum WS_OPCODE op_code, uint64_t payload_len, uint32_t mask)
{

    wss.tx_len = 0;
    wss.tx[wss.tx_len++] = op_code | (1 << 7);

    WSS_NEED_TX(payload_len + 16);

    if (payload_len <= 0x7d)
    {
        // 7 bit len
        wss.tx[wss.tx_len++] = payload_len | (1 << 7);
    }
    else if (payload_len <= 0xffffu)
    {
        payload_len &= 0xffff;
        wss.tx[wss.tx_len++] = 0x7E | (1 << 7);
        wss_write_int((uint8_t *)&payload_len, 2);
    }
    else
    {
        payload_len &= 0xffffffffffffffffLL;
        wss.tx[wss.tx_len++] = 0x7F | (1 << 7);
        wss_write_int((uint8_t *)&payload_len, 8);
    }
    warn("   payload %lu bytes", wss.tx_len - 2);

    // Masking
    *((uint32_t *)(&wss.tx[wss.tx_len])) = mask;
    //memcpy(wss.tx + wss.tx_len, (uint8_t *)&mask, 4);
    wss.tx_len += 4;
    return wss.tx_len;
}

static int wss_handshake(const char *host, const char *path)
{
    char nonce[16], ws_key[256], *cptr;
    int size, total = 0;

    crypto_random(nonce, 16);
    crypto_base64_encode(ws_key, 256, nonce, 16);

    wss.tx_len = sprintf(wss.tx, "GET %s HTTP/1.1\r\n"
                                 "Host: %s\r\n"
                                 "Origin: https://%s\r\n"
                                 "Upgrade: websocket\r\n"
                                 "Connection: Upgrade\r\n"
                                 "User-Agent: ndunks-whatsappd/1.0\r\n"
                                 "Sec-WebSocket-Key: %s\r\n"
                                 "Sec-WebSocket-Version: 13\r\n\r\n",
                         path, host, host, ws_key);
    do
    {
        size = ssl_write(&wss.tx[total], wss.tx_len - total);
        total += size;
    } while (total < wss.tx_len);

    /*
    HTTP/1.1 101 Switching Protocols
    Upgrade: websocket
    Connection: Upgrade
    Sec-WebSocket-Accept: 0AKLHEJZlxYFm0ZG4vzg0XdQn04=
    */
    wss.rx_len = 0;
    do
    {
        size = ssl_read(wss.rx, wss.rx_size);
        wss.rx[wss.rx_len + size] = 0;
        //fwrite(wss.rx + wss.rx_len, 1, size, stderr);
        cptr = strtok(wss.rx + wss.rx_len, "/");

        if (cptr != NULL)
        {
            // version
            cptr = strtok(NULL, " ");
            // code
            cptr = strtok(NULL, " \r\n");
            if (strncmp(cptr, "101", 3) != 0)
            {
                err("GOT HTTP CODE %s", cptr);
                return 1;
            }
            return 0;
        }

        wss.rx_len += size;
    } while (size > 0);

    err("No response from server");
    return 1;
}

int wss_connect(const char *host, const char *port, const char *path)
{

    memset(&wss, 0, sizeof(struct WSS));
    memset(&frame, 0, sizeof(struct FRAME));

    wss.rx_size = 1024 * 4;
    wss.tx_size = 1024 * 4;
    wss.rx = malloc(wss.rx_size);
    wss.tx = malloc(wss.tx_size);

    if (wss.rx == NULL || wss.tx == NULL)
    {
        err("wss: Fail alloc data buffer");
        CATCH_RET = 1;
        goto CATCH;
    }

    if (host == NULL)
        host = "web.whatsapp.com";

    if (path == NULL)
        path = "/ws";

    if (port == NULL)
        port = "443";

    TRY(ssl_connect(host, port));
    TRY(wss_handshake(host, path));

    return 0;

CATCH:
    free(wss.rx);
    free(wss.tx);
    return CATCH_RET;
}

int wss_disconnect()
{
    uint32_t mask = wss_mask();
    wss_frame(WS_OPCODE_CONNECTION, 0, mask);
    if (wss_send() != 6)
    {
        warn("wss: Send shutdown fail");
    }
    ssl_disconnect();
    free(wss.rx);
    free(wss.tx);
    return 0;
}

size_t wss_send_buffer(char *msg, size_t len, enum WS_OPCODE opcode)
{
    size_t frame_len;
    uint32_t mask = wss_mask();
    frame_len = wss_frame(opcode, len, mask);
    wss_write((uint8_t *)msg, len, (uint8_t *)&mask);
    return wss_send() - frame_len;
}

size_t wss_send_text(char *msg, size_t len)
{
    return wss_send_buffer(msg, len, WS_OPCODE_TEXT);
}

size_t wss_send_binary(char *msg, size_t len)
{
    return wss_send_buffer(msg, len, WS_OPCODE_BINARY);
}

void dump_frame()
{
    struct PAYLOAD *payload;
    info("   Frame %lu bytes, opcode: %d,payloads: %d",
         frame.payload_size,
         frame.opcode,
         frame.payload_count);

    for (int i = 0; i < frame.payload_count; i++)
    {
        payload = &frame.payloads[i];
        info("** payloads[%1$d]: size: %2$lu (0x%2$lx), frame: %3$d ", i, payload->size, payload->frame_size);
        if (payload->size < 256)
        {
            fwrite(payload->data, 1, payload->size, stderr);
            fprintf(stderr, "\n");
            hexdump(payload->data, payload->size);
        }
    }
    warn("\n------------------");
    // hexdump(wss.rx, wss.rx_len);
    // warn("\n==================");
}

char *wss_read(size_t *data_len)
{
    int recv;
    enum WS_OPCODE opcode;
    size_t offset = 0;
    ssize_t waiting_payload = 0;
    struct PAYLOAD *payload;
    uint8_t fin, masked;

    wss.rx_len = 0;
    frame.payload_count = 0;
    frame.payload_size = 0;
    payload = &frame.payloads[frame.payload_count];

    do
    {
        recv = ssl_read(wss.rx + wss.rx_len, wss.rx_size - wss.rx_len);
        ok("<< %d bytes", recv);
        wss.rx_len += recv;

        if (waiting_payload)
        {
            waiting_payload += recv;
            info("   WAIT %lu %d", waiting_payload, recv);
            if (waiting_payload == 0)
            { // received equal that we waiting
                goto process_payload;
            }
            else if (waiting_payload < 0)
            {
                ok("   need remaining payload %ld", waiting_payload);
                continue;
            }
            else
            {
                ok("   more frame here %ld", waiting_payload);
                offset += payload->frame_size + payload->size;
                recv = waiting_payload;
                payload = &frame.payloads[++frame.payload_count];
                goto process_frame;
            }
        }
        else
        {
            if (recv < 2)
            {
                warn("   Recv to small < 2");
                continue;
            }

        process_frame:

            fin = (wss.rx[offset] & 0x80) == 0x80; // (1 << 7)
            opcode = wss.rx[offset] & 0b1111;

            if (frame.payload_count == 0)
            {
                // set it on first frame
                // frame.rsv1 = (b & 0x40) == 0x40; // (1 << 6)
                // frame.rsv2 = (b & 0x20) == 0x20; // (1 << 5)
                // frame.rsv3 = (b & 0x10) == 0x10; // (1 << 4)
                frame.opcode = opcode;
                if (wss.rx[offset] & 0b01110000)
                {
                    warn("   GOT RSV BIT SET!");
                }
            }

            // assume no mask and payload_len 1 byte
            payload->frame_size = 2; // 0 = fin,etc, 1 = mask & len
            payload->size = wss.rx[offset + 1];

            masked = (payload->size & 0x80);
            if (masked)
            {
                // remove masked flag
                payload->size &= 0b1111111;
                warn("   GOT MASKED FRAME FROM SERVER!");
                payload->frame_size += 4; // mask is 4 byte
            }

            if (payload->size > 0x7d)
            {
                if (payload->size == 0x7E)
                {
                    payload->frame_size += 2;
                    payload->size = be16toh(*(uint16_t *)&wss.rx[offset + 2]);
                    ok("   payload->size (%2$d bytes) %1$ld 0x%1$02lX", payload->size, 2);
                }
                else if (payload->size == 0x7F)
                {
                    payload->frame_size += 8;
                    payload->size = be64toh(*(uint64_t *)&wss.rx[offset + 2]);
                    ok("   payload->size (%2$d bytes) %1$ld 0x%1$02lX", payload->size, 8);
                }
            }
        }

        //check all payload is completely received
        waiting_payload = wss.rx_len - (offset + payload->frame_size + payload->size);
        if (waiting_payload != 0)
        {
            ok("   Wait payload %ld", waiting_payload);
            WSS_NEED_RX((size_t)-waiting_payload);
            continue;
        }

    process_payload:
        // final data address
        payload->data = &wss.rx[offset + payload->frame_size];
        //hexdump(wss.rx + offset, payload->frame_size);
        ok("   payload: %lu, fin: %d, opode: %d", payload->size, fin, opcode);

        frame.payload_size += payload->size;
        payload = &frame.payloads[++frame.payload_count];
        memset(payload, 0, sizeof(struct PAYLOAD));
        offset += payload->frame_size + payload->size;

        if (frame.payload_count == WSS_FRAGMENT_MAX)
        {
            err("   Fragment to large!");
            break;
        }

        if (frame.opcode == WS_OPCODE_CONNECTION)
        {
            err("   Get close opcode");
            break;
        }

    } while (!fin || waiting_payload);

    ok("DONE: %d %d %ld", recv, fin, waiting_payload);
    dump_frame();

    // Merge payload into straight rx, removed the frame.

    offset = 0;
    for (recv = 0; recv < frame.payload_count; recv++)
    {
        payload = &frame.payloads[recv];
        memcpy(wss.rx + offset, payload->data, payload->size);
        offset += payload->size;
    }

    if (offset != frame.payload_size)
        err(" **payload_size not match! %lu != %lu", offset, frame.payload_size);

    *data_len = frame.payload_size;
    // Nulled
    wss.rx_len = offset;
    wss.rx[wss.rx_len] = 0;
    return wss.rx;
}