#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "binary_reader.h"

typedef enum BINARY_TAG
{
    STREAM_END = 2,
    LIST_EMPTY = 0,
    DICTIONARY_0 = 236,
    DICTIONARY_1 = 237,
    DICTIONARY_2 = 238,
    DICTIONARY_3 = 239,
    JID_AD = 247,
    LIST_8 = 248,
    LIST_16 = 249,
    JID_PAIR = 250,
    HEX_8 = 251,
    BINARY_8 = 252,
    BINARY_20 = 253,
    BINARY_32 = 254,
    NIBBLE_8 = 255,
    SINGLE_BYTE_MAX = 256,
    PACKED_MAX = 254
} BINARY_TAG;

char *DICTIONARY_SINGLEBYTE[];
int DICTIONARY_SINGLEBYTE_LEN;

char *binary_alloc(size_t size);
void binary_free();