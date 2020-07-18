#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <helper.h>
#include <color.h>

#include "wss.h"

#define WSS_FRAGMENT_MAX 100

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

static void wss_need_tx(size_t len)
{
    while ((wss.tx_len + len) > wss.tx_size)
    {
        wss.tx_size += 1024 * 4;
        wss.tx = realloc(wss.tx, wss.tx_size);
        if (wss.tx == NULL)
            die("wss: Fail realloc tx");
        warn("wss: realloc tx %lu", wss.tx_size);
    }
}

static void wss_need_rx(size_t len)
{
    while ((wss.rx_len + len) > wss.rx_size)
    {
        wss.rx_size += 1024 * 4;
        wss.rx = realloc(wss.rx, wss.rx_size);
        if (wss.rx == NULL)
            die("wss: Fail realloc rx");
        warn("wss: realloc rx %lu", wss.tx_size);
    }
}

static void wss_write(uint8_t *data, size_t len, uint8_t mask[4])
{
    size_t i;

    wss_need_tx(len);

    for (i = 0; i < len; i++)
    {
        wss.tx[wss.tx_len + i] = (data[i] ^ mask[i % 4]);
    }
    wss.tx_len += len;
}

static void wss_write_int(uint8_t *data, uint8_t len)
{
    wss_need_tx(len);

    for (int i = 0; i < len; i++)
    {
        //reversed
        wss.tx[wss.tx_len++] = data[len - (1 + i)];
    }
}

static void wss_read_int(uint8_t *dst, size_t offset, uint8_t len)
{
    for (int i = 0; i < len; i++)
    {
        //reversed
        dst[len - (1 + i)] = wss.rx[offset + i];
    }
}

static int wss_send()
{
    size_t sent, total = 0;
    warn(">> %lu bytes", wss.tx_len);
    hexdump(wss.tx, wss.tx_len);

    do
    {
        sent = ssl_write(&wss.tx[total], wss.tx_len - total);
        total += sent;
    } while (total < wss.tx_len && sent > 0);

    return total;
}

/* All control frames MUST have a payload length of 125 bytes or less
 * and MUST NOT be fragmented. */
// static int wss_handle_control_frame()
// {
//     return 0;
// }

// https://tools.ietf.org/html/rfc6455#page-31
static size_t wss_frame(enum WS_OPCODE op_code, uint64_t payload_len, uint32_t mask)
{

    wss.tx_len = 0;
    wss.tx[wss.tx_len++] = op_code | (1 << 7);

    // resize if not enough
    while ((wss.tx_size - 16) < payload_len)
    {
        wss.tx_size += 1024 * 4;
        warn("Realloc tx buffer %lu", wss.tx_size);
        wss.tx = realloc(wss.tx, wss.tx_size);
        if (wss.tx == NULL)
            die("Fail realloc tx buffer");
    }

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
    //info("TX: payload size %lu", payload_len);
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

size_t wss_send_text(char *msg, size_t len)
{
    size_t frame_len;
    uint32_t mask = wss_mask();
    frame_len = wss_frame(WS_OPCODE_TEXT, len, mask);
    wss_write((uint8_t *)msg, len, (uint8_t *)&mask);
    return wss_send() - frame_len;
}

void dump_frame()
{
    struct PAYLOAD *payload;
    warn("rsv1: %d\nrsv2: %d\nrsv3: %d\nopcode: %d\nmasked: %d\npayloads: %d\nmask: %08x",
         frame.rsv1,
         frame.rsv2,
         frame.rsv3,
         frame.opcode,
         frame.masked,
         frame.payload_count,
         frame.mask);

    // Null the last payload
    // payload = &frame.payloads[ frame.payload_count - 1 ];
    // payload->data[ payload->size ] = 0;

    for (int i = 0; i < frame.payload_count; i++)
    {
        payload = &frame.payloads[i];
        fwrite(payload->data, 1, payload->size, stderr);
        fprintf(stderr, "\n");
        hexdump(payload->data, payload->size);
    }
    warn("\n------------------");
    hexdump(wss.rx, wss.rx_len);
    warn("\n==================");
}

char *wss_read()
{
    int recv;
    enum WS_OPCODE opcode;
    size_t offset;
    ssize_t waiting_payload = 0;
    struct PAYLOAD *payload;
    uint8_t b, payload_bytes, fin, masked, frame_size;

    offset = 0;
    wss.rx_len = 0;
    frame.payload_count = 0;
    payload = &frame.payloads[frame.payload_count];

    do
    {
        recv = ssl_read(wss.rx + wss.rx_len, wss.rx_size - wss.rx_len);
        wss.rx_len += recv;
        ok("<< %d bytes", recv);

        if (waiting_payload)
        {
            waiting_payload += recv;
            info("WAIT %lu %d", waiting_payload, recv);
            if (waiting_payload == 0)
            { // received equal that we waiting
                goto process_payload;
            }
            else if (waiting_payload < 0)
            { // need remaining payload
                continue;
            }
            else
            { // more frame here
                offset += recv - waiting_payload;
                recv = waiting_payload;
                payload = &frame.payloads[++frame.payload_count];
                goto process_frame;
            }
        }
        else
        {
            if (recv < 2)
            {
                warn("Recv to small < 2");
                continue;
            }

        process_frame:
            b = wss.rx[offset];

            fin = (b & 0x80) == 0x80; // (1 << 7)
            opcode = b & 0b1111;

            if (frame.payload_count == 0)
            {
                // set it on first frame
                // frame.rsv1 = (b & 0x40) == 0x40; // (1 << 6)
                // frame.rsv2 = (b & 0x20) == 0x20; // (1 << 5)
                // frame.rsv3 = (b & 0x10) == 0x10; // (1 << 4)
                frame.opcode = opcode;
                if (b & 0b01110000)
                {
                    warn("GOT RSV BIT SET!");
                }
            }

            // assume no mask and payload_len 1 byte
            payload_bytes = 1;
            payload->frame_size = 1;
            payload->size = wss.rx[offset + 1];

            masked = (payload->size & 0x80);
            if (masked)
            {
                // remove masked flag
                payload->size &= 0b1111111;
                warn("GOT MASKED FRAME FROM SERVER!");
                payload->frame_size += 4; // mask is 4 byte
            }

            if (payload->size > 0x7d)
            {
                if (b == 0x7E)
                    payload_bytes = 2;
                else if (b == 0x7F)
                    payload_bytes = 8;

                wss_read_int(&(((uint8_t *)&payload->size)[8 - payload_bytes]), offset + 2, payload_bytes);
            }
            payload->frame_size += payload_bytes;
            payload->data = &wss.rx[offset + payload->frame_size];
        }

        //check all payload is completely received
        waiting_payload = wss.rx_len - (offset + payload->frame_size + payload->size);
        if (waiting_payload != 0)
        {
            //warn("Wait payload %d %d", waiting_payload, -waiting_payload);
            wss_need_rx(-waiting_payload);
            continue;
        }

    process_payload:
        // final data address
        ok("payload: %lu, fin: %d, opode: %d", payload->size, fin, opcode);
        payload = &frame.payloads[++frame.payload_count];
        memset(payload, 0, sizeof(struct PAYLOAD));
        offset = wss.rx_len;

        if (frame.payload_count == WSS_FRAGMENT_MAX)
        {
            err("Fragment to large!");
            break;
        }

        if (frame.opcode == WS_OPCODE_CONNECTION)
        {
            err("Get close opcode");
            break;
        }

    } while (!fin || waiting_payload);

    warn("DONE: %d %d %ld", recv, fin, waiting_payload);
    dump_frame();
    // Merge payload into straight rx, removed the frame.
    frame.payload_size = 0;
    for (recv = 0; recv < frame.payload_count; recv++)
    {
        payload = &frame.payloads[recv];
        memcpy(wss.rx + frame.payload_size, payload->data, payload->size);
        frame.payload_size += payload->size;
    }

    // Nulled
    wss.rx_len = frame.payload_size;
    wss.rx[wss.rx_len] = 0;
    return wss.rx;
}