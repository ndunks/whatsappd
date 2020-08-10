#include <locale.h>
#include <sys/file.h>
#include <unistd.h>

#include "util.h"
#include "qrcodegen.h"

static char helper_hex[] = "0123456789ABCDEF", helper_epoch_str[9] = "";
static uint32_t helper_epoch_num = 0;

int CATCH_RET = 0;

char *helper_epoch()
{
    sprintf(helper_epoch_str, "%u", ++helper_epoch_num);
    return helper_epoch_str;
}

void helper_buf_to_hex(const uint8_t *dst, uint8_t *buf, int buf_len)
{
    uint8_t *ptr = (uint8_t *)dst;
    for (int i = 0; i < buf_len; i++)
    {
        *(ptr++) = helper_hex[buf[i] >> 4];
        *(ptr++) = helper_hex[buf[i] & 0b1111];
    }
    *ptr = 0;
}

int helper_save_file(const char *path, const char *buf, size_t buf_len)
{
    int file = open(path, O_WRONLY | O_CREAT, 0644);
    if (file < 0)
    {
        err("Fail open write %s", path);
        return -1;
    }

    if (write(file, buf, buf_len) != buf_len)
    {
        err("Fail write %s", path);
        close(file);
        return 1;
    };
    close(file);
    return 0;
}

uint64_t helper_jid_to_num(const char *buf)
{
    int len;
    char *end, tmp[21] = {0};
    end = strrchr(buf, '@');

    if (end == NULL)
        len = strlen(buf);
    else
        len = end - buf;

    if (len > 21)
    {
        err("JID too long");
        return 0;
    }
    strncpy(tmp, buf, len);
    tmp[len] = 0;
    return atoll(tmp);
}

int helper_qrcode_show(const char *src)
{
    int size, border = 2;
    bool top, bot;
    const char *ptr,
        black_white[] = "\xe2\x96\x84", // U+0x2584
        white_all[] = "\xe2\x96\x88",   // U+0x2588
        white_black[] = "\xe2\x96\x80", // U+0x2580
        black_all[] = " ";

    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX],
        tempBuffer[qrcodegen_BUFFER_LEN_MAX];

    if (!qrcodegen_encodeText(src, tempBuffer, qrcode,
                              qrcodegen_Ecc_LOW, 5, 16,
                              qrcodegen_Mask_AUTO, true))
    {
        return 1;
    };

    size = qrcodegen_getSize(qrcode);
    setlocale(LC_CTYPE, "");

    for (int y = -border; y < size + border; y += 2)
    {
        for (int x = -border; x < size + border; x++)
        {
            top = qrcodegen_getModule(qrcode, x, y);
            bot = qrcodegen_getModule(qrcode, x, y + 1);
            if (top && bot)
                ptr = white_all;
            else if (top)
                ptr = white_black;
            else if (bot)
                ptr = black_white;
            else
                ptr = black_all;
            fputs(ptr, stdout);
        }
        fputs("\n", stdout);
    }
    fputs("\n", stdout);
    return 0;
}
