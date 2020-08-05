#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "helper.h"

#define BUF_ERR -1
#define BUF_ERR_NOT_ENOUGH -2

extern uint8_t *buf;
extern size_t buf_idx, buf_len;

void buf_set(char *src, size_t src_len);
uint8_t buf_read_byte();
// Big-endian read
uint16_t buf_read_int16();
// Big-endian read
uint32_t buf_read_int20();
// Big-endian read
uint32_t buf_read_int32();

uint32_t buf_read_var_int32(unsigned len, const uint8_t *data);
void buf_read_bytes(char *dst, size_t len);
bool buf_available();