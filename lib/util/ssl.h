#pragma once

int ssl_random(char *buf, size_t len);
int ssl_write(const char *buf, size_t size);
int ssl_read(char *buf, size_t size);
void ssl_disconnect();
int ssl_connect(const char *host, const char *port);