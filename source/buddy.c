#include "buddy.h"
#include "bitmap.h"

#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void buddy_init(BuddyAllocator* buddy) {
    // Allocate 1MB memory pool using mmap
    buddy->memory_pool = mmap(NULL, 1024 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buddy->memory_pool == MAP_FAILED) {
        perror("Failed to allocate memory pool");
        exit(EXIT_FAILURE);
    }

    // Calculate the number of bits needed for split and allocation bitmaps
    uint32_t split_bits_num = 1023;  // n - 1 = 1023
    uint32_t alloc_bits_num = 2047;  // 2n - 1 = 2047

    // Calculate buffer sizes for the bitmaps
    uint32_t split_buffer_size = BitMap_getBytes(split_bits_num);
    uint32_t alloc_buffer_size = BitMap_getBytes(alloc_bits_num);

    // Allocate buffers for split and allocation bitmaps using mmap
    uint8_t* split_buffer = mmap(NULL, split_buffer_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (split_buffer == MAP_FAILED) {
        perror("Failed to allocate split bitmap buffer");
        exit(EXIT_FAILURE);
    }

    uint8_t* alloc_buffer = mmap(NULL, alloc_buffer_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (alloc_buffer == MAP_FAILED) {
        perror("Failed to allocate allocation bitmap buffer");
        exit(EXIT_FAILURE);
    }

    // Initialize the split and allocation bitmaps
    bitmap_init(&buddy->split_bits, split_buffer, split_bits_num);
    bitmap_init(&buddy->alloc_bits, alloc_buffer, alloc_bits_num);

    // Set the minimum block size to 1KB
    buddy->min_block_size = 1024;
}

// Returns the level (0 = 1MB, 10 = 1KB) for a given block size
uint32_t get_level(uint32_t block_size) {
    uint32_t log2_block_size = 0;
    for (uint32_t tmp = block_size; tmp > 1; tmp >>= 1) log2_block_size++;
    return 20 - log2_block_size;  // 20 = log2(1MB)
}

// Finds the first free block index at the specified level
int32_t find_free_block(BuddyAllocator* buddy, uint32_t level) {
    uint32_t start = (1 << level) - 1;
    uint32_t end = (1 << (level + 1)) - 1;
    for (uint32_t i = start; i < end; i++) {
        // Skip allocated or split blocks
        if (bitmap_is_set(&buddy->alloc_bits, i) || 
            (level != 10 && bitmap_is_set(&buddy->split_bits, i))) {
            continue;
        }

        // Check if any ancestor is allocated (up to root)
        int is_free = 1;
        uint32_t current = i;
        for (uint32_t l = level; l > 0; l--) {
            uint32_t parent = (current - 1) / 2;
            if (parent >= buddy->alloc_bits.num_bits || 
                bitmap_is_set(&buddy->alloc_bits, parent)) {
                is_free = 0;
                break;
            }
            current = parent;
        }
        if (is_free) return i;
    }
    return -1;
}

// Splits a block recursively from `current_level` down to `target_level`
void split_block(BuddyAllocator* buddy, uint32_t index, 
                  uint32_t current_level, uint32_t target_level) {
    for (uint32_t l = current_level; l < target_level; l++) {
        if (index >= buddy->split_bits.num_bits) break; // Guard split bitmap
        bitmap_set(&buddy->split_bits, index);
        index = 2 * index + 1; // Move to left child
    }
}

// Finds the block index and level for a given memory offset
int32_t find_block_index(BuddyAllocator* buddy, uint32_t offset, uint32_t* out_level) {
    for (int32_t l = 10; l >= 0; l--) {
        uint32_t block_size = 1048576 >> l;
        if (block_size < 1024) continue;
        if (offset % block_size != 0) continue;

        uint32_t start = (1 << l) - 1;
        uint32_t index = start + (offset / block_size);
        if (index < (uint32_t)((1 << (l + 1)) - 1) && 
            bitmap_is_set(&buddy->alloc_bits, index)) {
            *out_level = l;
            return index;
        }
    }
    return -1;
}

// Merges free buddies upwards recursively
void merge_buddies(BuddyAllocator* buddy, uint32_t index, uint32_t level) {
    while (level < 10) {
        uint32_t parent_index = (index - 1) / 2;
        if (parent_index >= buddy->split_bits.num_bits) break; // Guard

        uint32_t buddy_index = index ^ 1;
        if (bitmap_is_set(&buddy->alloc_bits, buddy_index) || 
            bitmap_is_set(&buddy->split_bits, parent_index)) {
            break; // Buddy is allocated or parent is split
        }

        bitmap_clear(&buddy->split_bits, parent_index);
        index = parent_index;
        level--;
    }
}

void* buddy_alloc(BuddyAllocator* buddy, uint32_t size) {
    if (size == 0 || size > 1048576) return NULL;

    // Calculate required block size (round up to nearest power of 2)
    uint32_t block_size = 1024;
    if (size > 1024) {
        block_size = 1;
        while (block_size < size) block_size <<= 1;
        if (block_size > 1048576) return NULL;
    }
    uint32_t target_level = get_level(block_size);

    // Try to allocate directly at target level
    int32_t index = find_free_block(buddy, target_level);
    if (index != -1) {
        bitmap_set(&buddy->alloc_bits, index);
        uint32_t offset = (index - ((1 << target_level) - 1)) * block_size;
        return buddy->memory_pool + offset;
    }

    // Search higher levels and split
    for (int32_t current_level = target_level - 1; current_level >= 0; current_level--) {
        index = find_free_block(buddy, current_level);
        if (index != -1) {
            split_block(buddy, index, current_level, target_level);
            uint32_t splits = target_level - current_level;
            uint32_t final_index = (index << splits) | ((1 << splits) - 1);
            if (final_index >= buddy->alloc_bits.num_bits) return NULL; // Guard
            bitmap_set(&buddy->alloc_bits, final_index);
            uint32_t offset = (final_index - ((1 << target_level) - 1)) * block_size;
            return buddy->memory_pool + offset;
        }
    }

    return NULL; // Out of memory
}

void buddy_free(BuddyAllocator* buddy, void* ptr) {
    if (ptr == NULL || 
        (uintptr_t)ptr < (uintptr_t)buddy->memory_pool || 
        (uintptr_t)ptr >= (uintptr_t)(buddy->memory_pool + 1048576)) {
        return;
    }

    uint32_t offset = (uint8_t*)ptr - buddy->memory_pool;
    uint32_t level;
    int32_t index = find_block_index(buddy, offset, &level);
    if (index == -1) return;

    // Check double free
    if (!bitmap_is_set(&buddy->alloc_bits, index)) {
        return;
    }

    bitmap_clear(&buddy->alloc_bits, index);
    merge_buddies(buddy, index, level);
}