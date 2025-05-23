#include "bitmap.h"

#include <assert.h>
#include <stdint.h>

// Calculates the number of bytes required to store a given number of bits.
uint32_t BitMap_getBytes(uint32_t bits){
    return (bits + 7) / 8;
}

// Initializes a BitMap structure with the given buffer and number of bits.
void bitmap_init(BitMap* bm, uint8_t* buffer, uint32_t num_bits){
    bm->buffer = buffer;
    bm->buffer_size = BitMap_getBytes(num_bits);
    bm->num_bits = num_bits;
}

// Sets (marks as 1) the bit at the specified index in the bitmap.
void bitmap_set(BitMap* bm, uint32_t index){
    assert(index < bm->num_bits);
    uint32_t byte_index = index / 8;
    uint8_t bit_position = index % 8;
    uint8_t mask = 1 << bit_position;
    bm->buffer[byte_index] |= mask;
}

// Clears (marks as 0) the bit at the specified index in the bitmap.
void bitmap_clear(BitMap* bm, uint32_t index){
    assert(index < bm->num_bits);
    uint32_t byte_index = index / 8;
    uint8_t bit_position = index % 8;
    uint8_t mask = 1 << bit_position;
    bm->buffer[byte_index] &= ~mask;
}

// Checks if the bit at the specified index is set (1).
int bitmap_is_set(const BitMap* bm, uint32_t index){
    assert(index < bm->num_bits);
    uint32_t byte_index = index / 8;
    uint8_t bit_position = index % 8;
    uint8_t mask = 1 << bit_position;
    return (bm->buffer[byte_index] & mask) ? 1 : 0;
}
