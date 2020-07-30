#pragma once

#include <stdlib.h>
#include <stdint.h>

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
