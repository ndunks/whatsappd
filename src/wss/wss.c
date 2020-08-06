#include <malloc.h>
#include <string.h>
#include <time.h>
#include <byteswap.h>
#include <mbedtls/net_sockets.h>

#include "wss.h"

WSS wss;
FRAME_RX wss_frame_rx;

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

void wss_write_chunk(uint8_t *src, size_t src_start, size_t src_end, const uint32_t *const mask)
{
    size_t i, len, pos = 0;
    len = src_end - src_start;

    WSS_NEED_TX(len);

    for (i = src_start; i < src_end; (i++, pos++))
    {
        wss.tx[wss.tx_len + pos] = (src[pos] ^ (((uint8_t *)mask)[i % 4]));
    }
    wss.tx_len += len;
}

void wss_write(uint8_t *src, size_t len, const uint32_t *const mask)
{
    wss_write_chunk(src, 0, len, mask);
}

int wss_send()
{
    size_t sent, total = 0;
    // if (wss.tx_len < 255)
    // {
    //     hexdump(wss.tx, wss.tx_len);
    // }

    do
    {
        sent = wss_ssl_write(&wss.tx[total], wss.tx_len - total);
        warn(">> %lu bytes", sent);
        total += sent;
    } while (total < wss.tx_len && sent > 0);
    return total;
}

// https://tools.ietf.org/html/rfc6455#page-31
size_t wss_frame(enum WS_OPCODE op_code, uint64_t payload_len, uint32_t mask)
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
        *((uint16_t *)&wss.tx[wss.tx_len]) = htobe16(payload_len);
        wss.tx_len += 2;
    }
    else
    {
        payload_len &= 0xffffffffffffffffLL;
        wss.tx[wss.tx_len++] = 0x7F | (1 << 7);
        *((uint64_t *)&wss.tx[wss.tx_len]) = htobe64(payload_len);
        wss.tx_len += 8;
    }
    warn("   payload %lu bytes", wss.tx_len - 2);

    // Masking
    *((uint32_t *)(&wss.tx[wss.tx_len])) = mask;
    //memcpy(wss.tx + wss.tx_len, &mask, 4);
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
        size = wss_ssl_write(&wss.tx[total], wss.tx_len - total);
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
        size = wss_ssl_read(wss.rx, wss.rx_size);
        wss.rx[wss.rx_len + size] = 0;
        //fwrite(wss.rx + wss.rx_len, 1, size, stderr);
        cptr = strtok(wss.rx + wss.rx_len, "/");

        if (cptr != NULL)
        {
            cptr = strtok(NULL, " ");     // version
            cptr = strtok(NULL, " \r\n"); // code

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
    memset(&wss_frame_rx, 0, sizeof(struct FRAME_RX));

    wss.rx_size = 1024 * 10;
    wss.tx_size = 1024 * 50;
    wss.buf_size = 1024 * 10;
    wss.rx = malloc(wss.rx_size);
    wss.tx = malloc(wss.tx_size);
    wss.buf = malloc(wss.buf_size);

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

    TRY(wss_ssl_connect(host, port));
    TRY(wss_handshake(host, path));

    return 0;

CATCH:
    free(wss.rx);
    free(wss.tx);
    free(wss.buf);
    return CATCH_RET;
}

void wss_disconnect()
{
    uint32_t mask = wss_mask();
    wss_frame(WS_OPCODE_CONNECTION, 0, mask);

    if (wss_send() != 6)
        warn("wss: Send shutdown fail");

    wss_ssl_disconnect();
    free(wss.rx);
    free(wss.tx);
    free(wss.buf);
}

size_t wss_send_buffer(char *msg, size_t len, enum WS_OPCODE opcode)
{
    size_t frame_len;
    uint32_t mask = wss_mask();
    frame_len = wss_frame(opcode, len, mask);
    wss_write((uint8_t *)msg, len, &mask);
    return wss_send() - frame_len;
}

size_t wss_send_text(char *msg, size_t len)
{
    if (len < 255)
    {
        warn(">> %lu bytes (text)\n%s", len, msg);
    }
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
         wss_frame_rx.payload_size,
         wss_frame_rx.opcode,
         wss_frame_rx.payload_count);

    for (int i = 0; i < wss_frame_rx.payload_count; i++)
    {
        payload = &wss_frame_rx.payloads[i];
        info("** payloads[%1$d]: size: %2$lu (0x%2$lx), frame: %3$d ", i, payload->size, payload->frame_size);
        // if (payload->size < 1024)
        // {
        //     fwrite(payload->data, 1, payload->size, stderr);
        //     fprintf(stderr, "\n");
        //     hexdump(payload->data, payload->size);
        // }
    }
    warn("\n------------------");
    // hexdump(wss.rx, wss.rx_len);
    // warn("\n==================");
}

char *wss_read(size_t *data_len)
{
    int recv, i;
    enum WS_OPCODE opcode;
    uint64_t offset = 0;
    ssize_t waiting_payload = 0;
    struct PAYLOAD *payload;
    uint8_t fin, masked;
    size_t total_size = 0;

    wss.rx_len = 0;
    wss_frame_rx.payload_count = 0;
    wss_frame_rx.payload_size = 0;
    payload = &wss_frame_rx.payloads[0];

    do
    {
        if (wss.buf_len > 0)
        {
            accent(" * Restored buffer");
            memcpy(wss.rx + wss.rx_len, wss.buf, wss.buf_len);
            recv = wss.buf_len;
            wss.buf_len = 0;
        }
        else
            recv = wss_ssl_read(wss.rx + wss.rx_len, wss.rx_size - wss.rx_len);
        ok("<< %d bytes", recv);

        if (recv < 0)
        {
            wss_ssl_error("wss_read", recv);
            return NULL;
        }

        wss.rx_len += recv;

        if (waiting_payload)
        {
            waiting_payload += recv;
            if (waiting_payload == 0)
            { // received equal that we waiting
                goto process_payload;
            }
            else if (waiting_payload < 0)
            {
                ok("   need payload %ld", waiting_payload);
                continue;
            }
            else
            {
                recv = waiting_payload;
                waiting_payload = 0;

                if (fin)
                {
                    accent("   another frame, save it %d", recv);
                    WSS_NEED_BUF(recv);
                    // Copy it to our buffers
                    memcpy(
                        &wss.buf[wss.buf_len],
                        &wss.rx[total_size],
                        recv);
                    wss.buf_len += recv;
                    goto process_payload;
                }
                else
                { // next fragment
                    accent("   next fragment %d", recv);
                    offset += total_size;
                    payload = &wss_frame_rx.payloads[++wss_frame_rx.payload_count];
                    total_size = 0;
                    goto process_frame;
                }
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
            accent(" * fin: %d, opode: %d", fin, opcode);

            if (wss_frame_rx.payload_count == 0)
            {
                wss_frame_rx.opcode = opcode;
                if (wss.rx[offset] & 0b01110000)
                    warn("   GOT RSV BIT SET!");
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

        total_size = payload->frame_size + payload->size;

        //check all payload is completely received
        waiting_payload = wss.rx_len - (offset + total_size);
        if (waiting_payload < 0)
        {
            ok("   Wait payload %ld", waiting_payload);
            WSS_NEED_RX((size_t)-waiting_payload);
            continue;
        }
        if (waiting_payload > 0)
        {
            accent("   too many bytes, save it %ld", waiting_payload);
            WSS_NEED_BUF(waiting_payload);
            // Copy it to our buffers
            memcpy(
                &wss.buf[wss.buf_len],
                &wss.rx[total_size],
                waiting_payload);
            wss.buf_len += waiting_payload;
            wss.rx_len = offset + total_size;
            waiting_payload = 0;
        }

    process_payload:
        // final data address
        payload->data = &wss.rx[offset + payload->frame_size];
        ok("   payload: count: %u, size %lu", wss_frame_rx.payload_count, payload->size);

        wss_frame_rx.payload_size += payload->size;
        payload = &wss_frame_rx.payloads[++wss_frame_rx.payload_count];
        memset(payload, 0, sizeof(struct PAYLOAD));
        offset += total_size;

        if (wss_frame_rx.payload_count == WSS_FRAGMENT_MAX)
        {
            err("   Fragment to large!");
            break;
        }

        if (wss_frame_rx.opcode == WS_OPCODE_CONNECTION)
        {
            err("   Get close opcode");
            break;
        }

    } while (!fin || waiting_payload);

    dump_frame();

    // Merge payload into straight rx, removed the wss_frame_rx.
    offset = 0;
    for (i = 0; i < wss_frame_rx.payload_count; i++)
    {
        payload = &wss_frame_rx.payloads[i];
        memcpy(wss.rx + offset, payload->data, payload->size);
        offset += payload->size;
    }

    if (offset != wss_frame_rx.payload_size)
        err(" **payload_size not match! %lu != %lu", offset, wss_frame_rx.payload_size);

    if (data_len != NULL)
        *data_len = wss_frame_rx.payload_size;
    // Nulled
    wss.rx_len = offset;
    wss.rx[wss.rx_len] = 0;
    return wss.rx;
}