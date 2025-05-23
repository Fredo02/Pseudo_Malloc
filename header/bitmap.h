#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

typedef struct {
    uint8_t* buffer;        // Externally allocated buffer
    uint32_t buffer_size;   // Size of the buffer in BYTES
    uint32_t num_bits;      // Number of bits the bitmap can track
} BitMap;

// returns the number of bytes to store bits booleans
uint32_t BitMap_getBytes(uint32_t bits);

// Initialize the bitmap with a pre-allocated buffer
void bitmap_init(BitMap* bm, uint8_t* buffer, uint32_t num_bits);

// Bit operations
void bitmap_set(BitMap* bm, uint32_t index);
void bitmap_clear(BitMap* bm, uint32_t index);
int  bitmap_is_set(const BitMap* bm, uint32_t index);

#endif