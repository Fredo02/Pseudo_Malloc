#ifndef BUDDY_H
#define BUDDY_H

#include "bitmap.h"

// Buddy allocator managing a 1MB memory pool
typedef struct {
    uint8_t* memory_pool;       // 1MB pool for small allocations
    BitMap split_bits;          // Tracks split blocks (1 = split)
    BitMap alloc_bits;          // Tracks allocated blocks (1 = allocated)
    uint32_t min_block_size;    // 1024 bytes (1KB)
} BuddyAllocator;

// Initialize the buddy allocator with mmap-ed memory
void buddy_init(BuddyAllocator* buddy);

// Allocate/free memory from the buddy system
void* buddy_alloc(BuddyAllocator* buddy, uint32_t size);
void buddy_free(BuddyAllocator* buddy, void* ptr);

// Auxiliary functions
uint32_t get_level(uint32_t block_size);
int32_t find_free_block(BuddyAllocator* buddy, uint32_t level);
void split_block(BuddyAllocator* buddy, uint32_t index, uint32_t current_level, uint32_t target_level);
int32_t find_block_index(BuddyAllocator* buddy, uint32_t offset, uint32_t* level);
void merge_buddies(BuddyAllocator* buddy, uint32_t index, uint32_t level);

#endif