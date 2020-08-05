#pragma once
#include "binary.h"

BINARY_NODE *read_node();
BINARY_NODE *binary_read(char *src, size_t src_len);

#ifdef TEST
uint8_t read_byte();
uint16_t read_int16();
uint32_t read_int20();
uint32_t read_int32();
void binary_read_set(char *src, size_t src_len);
#endif