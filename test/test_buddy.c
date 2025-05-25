#include "buddy.h"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

// Test initialization of the BuddyAllocator
void test_init() {
    BuddyAllocator buddy;
    buddy_init(&buddy);

    assert(buddy.memory_pool != NULL);
    assert(buddy.split_bits.buffer != NULL);
    assert(buddy.alloc_bits.buffer != NULL);
    assert(buddy.min_block_size == 1024);

    // Verify split and alloc bitmap sizes
    assert(buddy.split_bits.num_bits == 1023);
    assert(buddy.alloc_bits.num_bits == 2047);

    printf("test_init passed\n");
}

// Test single allocation and free
void test_simple_alloc_free() {
    BuddyAllocator buddy;
    buddy_init(&buddy);

    void* ptr = buddy_alloc(&buddy, 512); // Allocates 1KB
    assert(ptr != NULL);

    uint32_t offset = (uint8_t*)ptr - buddy.memory_pool;
    assert(offset % 1024 == 0); // Check alignment

    // Verify allocation bit is set
    uint32_t index = (1 << 10) - 1 + (offset / 1024);
    assert(bitmap_is_set(&buddy.alloc_bits, index));

    buddy_free(&buddy, ptr);
    assert(!bitmap_is_set(&buddy.alloc_bits, index));

    printf("test_simple_alloc_free passed\n");
}

// Test splitting and merging of blocks
void test_split_and_merge() {
    BuddyAllocator buddy;
    buddy_init(&buddy);

    void* ptr = buddy_alloc(&buddy, 2048); // Allocate 2KB
    assert(ptr != NULL);

    // Check split bits for ancestors
    uint32_t split_indices[] = {0, 1, 3, 7, 15, 31, 63, 127, 255};
    for (int i = 0; i < 9; i++) {
        assert(bitmap_is_set(&buddy.split_bits, split_indices[i]));
    }

    buddy_free(&buddy, ptr);

    // Verify split bits are cleared after merge
    for (int i = 0; i < 9; i++) {
        assert(!bitmap_is_set(&buddy.split_bits, split_indices[i]));
    }

    printf("test_split_and_merge passed\n");
}

// Test exhausting and freeing all memory
void test_exhaust_memory() {
    BuddyAllocator buddy;
    buddy_init(&buddy);

    void* pointers[1024];
    for (int i = 0; i < 1024; i++) {
        pointers[i] = buddy_alloc(&buddy, 1024); // Allocate all 1KB blocks
        assert(pointers[i] != NULL);
    }

    // Next allocation should fail
    void* ptr = buddy_alloc(&buddy, 1024);
    assert(ptr == NULL);

    // Free all blocks
    for (int i = 0; i < 1024; i++) {
        buddy_free(&buddy, pointers[i]);
    }

    // Allocation should succeed again
    ptr = buddy_alloc(&buddy, 1024);
    assert(ptr != NULL);
    buddy_free(&buddy, ptr);

    printf("test_exhaust_memory passed\n");
}

// Test alignment of allocated blocks
void test_alignment() {
    BuddyAllocator buddy;
    buddy_init(&buddy);

    // Check alignment for various block sizes
    void* ptr1 = buddy_alloc(&buddy, 1024);
    assert(ptr1 != NULL);
    assert((uintptr_t)ptr1 % 1024 == 0);

    void* ptr2 = buddy_alloc(&buddy, 2048);
    assert(ptr2 != NULL);
    assert((uintptr_t)ptr2 % 2048 == 0);

    void* ptr3 = buddy_alloc(&buddy, 4096);
    assert(ptr3 != NULL);
    assert((uintptr_t)ptr3 % 4096 == 0);

    buddy_free(&buddy, ptr1);
    buddy_free(&buddy, ptr2);
    buddy_free(&buddy, ptr3);

    printf("test_alignment passed\n");
}

// Test invalid allocation and free operations
void test_invalid_allocs() {
    BuddyAllocator buddy;
    buddy_init(&buddy);

    // Allocate 0 bytes
    assert(buddy_alloc(&buddy, 0) == NULL);

    // Allocate more than 1MB
    assert(buddy_alloc(&buddy, 2 * 1024 * 1024) == NULL);

    void* ptr = buddy_alloc(&buddy, 1024);
    assert(ptr != NULL);

    // Free invalid addresses (outside pool)
    buddy_free(&buddy, (void*)(buddy.memory_pool - 1));
    buddy_free(&buddy, (void*)(buddy.memory_pool + 1024 * 1024));

    // Free valid address
    buddy_free(&buddy, ptr);

    printf("test_invalid_allocs passed\n");
}

int main() {
    test_init();
    test_simple_alloc_free();
    test_split_and_merge();
    test_exhaust_memory();
    test_alignment();
    test_invalid_allocs();

    printf("All tests passed!\n");
    return 0;
}