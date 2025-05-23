#include "bitmap.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test BitMap_getBytes calculations
void test_bitmap_getbytes() {
    assert(BitMap_getBytes(0) == 0);   // Edge case: 0 bits
    assert(BitMap_getBytes(1) == 1);   // 1 bit needs 1 byte
    assert(BitMap_getBytes(7) == 1);   // 7 bits → 1 byte
    assert(BitMap_getBytes(8) == 1);   // Exact byte
    assert(BitMap_getBytes(9) == 2);   // 9 bits → 2 bytes
    assert(BitMap_getBytes(15) == 2);  // 15 bits → 2 bytes
    assert(BitMap_getBytes(16) == 2);  // 16 bits → 2 bytes
    assert(BitMap_getBytes(17) == 3);  // 17 bits → 3 bytes
    printf("test_bitmap_getbytes passed!\n");
}

// Test bitmap_init initializes struct correctly
void test_bitmap_init() {
    uint8_t buffer[4];
    BitMap bm;
    bitmap_init(&bm, buffer, 25);  // 25 bits need 4 bytes (25+7)/8=4

    assert(bm.buffer == buffer);
    assert(bm.buffer_size == 4);
    assert(bm.num_bits == 25);
    printf("test_bitmap_init passed!\n");
}

// Test setting, clearing, and checking individual bits
void test_bit_operations() {
    uint8_t buffer[2] = {0};  // 2 bytes = 16 bits
    BitMap bm;
    bitmap_init(&bm, buffer, 10);  // Use 10 bits (2 bytes)

    // Test setting/clearing bit 3
    bitmap_set(&bm, 3);
    assert(bitmap_is_set(&bm, 3) == 1);
    assert(buffer[0] == 0x08);  // 00001000

    bitmap_clear(&bm, 3);
    assert(bitmap_is_set(&bm, 3) == 0);
    assert(buffer[0] == 0x00);

    // Test setting bit 8 (second byte)
    bitmap_set(&bm, 8);
    assert(bitmap_is_set(&bm, 8) == 1);
    assert(buffer[1] == 0x01);  // 00000001

    // Ensure adjacent bits are unaffected
    assert(bitmap_is_set(&bm, 7) == 0);  // Last bit of first byte
    assert(bitmap_is_set(&bm, 9) == 0);  // Next bit in second byte

    printf("test_bit_operations passed!\n");
}

// Test all bits in a small bitmap
void test_all_bits() {
    uint8_t buffer[2] = {0};
    BitMap bm;
    bitmap_init(&bm, buffer, 16);  // 16 bits (2 bytes)

    // Set all bits
    for (uint32_t i = 0; i < 16; i++) {
        bitmap_set(&bm, i);
        assert(bitmap_is_set(&bm, i) == 1);
    }

    // Verify all bits are set
    assert(buffer[0] == 0xFF);  // 11111111
    assert(buffer[1] == 0xFF);

    // Clear all bits
    for (uint32_t i = 0; i < 16; i++) {
        bitmap_clear(&bm, i);
        assert(bitmap_is_set(&bm, i) == 0);
    }

    assert(buffer[0] == 0x00);
    assert(buffer[1] == 0x00);
    printf("test_all_bits passed!\n");
}

// Test edge cases (first and last bit in the bitmap)
void test_edge_cases() {
    uint8_t buffer[3] = {0};
    BitMap bm;
    bitmap_init(&bm, buffer, 17);  // 17 bits (3 bytes)

    // First bit (index 0)
    bitmap_set(&bm, 0);
    assert(buffer[0] == 0x01);     // 00000001
    bitmap_clear(&bm, 0);
    assert(buffer[0] == 0x00);

    // Last bit (index 16)
    bitmap_set(&bm, 16);
    assert(buffer[2] == 0x01);     // Corrected: 00000001 (LSB of third byte)
    assert(bitmap_is_set(&bm, 16) == 1);
    bitmap_clear(&bm, 16);
    assert(buffer[2] == 0x00);

    printf("test_edge_cases passed!\n");
}

int main() {
    test_bitmap_getbytes();
    test_bitmap_init();
    test_bit_operations();
    test_all_bits();
    test_edge_cases();

    printf("All bitmap tests passed!\n");
    return 0;
}