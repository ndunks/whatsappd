#pragma once

#include <mbedtls/net_sockets.h>
#include "crypto.h"

int ssl_connect(const char *host, const char *port);
void ssl_disconnect();

int ssl_random(char *buf, size_t len);
int ssl_write(const char *buf, size_t size);
int ssl_read(char *buf, size_t size);
int ssl_check_read(uint32_t timeout_ms);
void ssl_error(const char *msg, int errcode);