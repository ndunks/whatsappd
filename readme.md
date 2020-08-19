# The Whatsapp Daemon

Using staticaly linked [mbedtls](https://github.com/ARMmbed/mbedtls) as crypto engine and HTTPS/TLS 1.2 Client and [Nayuki's QRCode gen](https://github.com/nayuki/QR-Code-generator).

## Build & Run

``` bash
# Build
make
# run 
./build/whatsappd
```

- You will be prompted to login using QR Code.
- Configration stored in `~/.whatsappd.cfg`. 
- You only need to login once. If you need to relogin, just delete the config.
- whatsappd will creating fifo file in `/dev/shm/whatsappd` to send message from terminal.

### Sending message

You can send message from a terminal.

Format: `number message`

Example:

``` bash
# send message
echo "6285726501017 You got message from $USER" > /dev/shm/whatsappd
# result should be "2" (SENDER_RESULT_OK) on success
cat /dev/shm/whatsappd
```

## Refferences

- WhatsApp information from: [sigalor/whatsapp-web-reveng](https://github.com/sigalor/whatsapp-web-reveng)
- WhatsApp rule & logic from [ndunks/WaJs](https://github.com/ndunks/WaJs)
- Some Websocket logic code stealed from here:
[morrowind/libwsclient](https://github.com/morrowind/libwsclient/)
- Websocket Refference: [RFC 6455](https://tools.ietf.org/html/rfc6455)

