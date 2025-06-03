#include "buddy.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stddef.h>

// System page size (typically 4096 bytes)
#define PAGE_SIZE 4096
// Threshold between small and large allocations (1/4 page)
#define SMALL_THRESHOLD (PAGE_SIZE / 4)

// Global buddy allocator instance
static BuddyAllocator global_buddy;
static int buddy_initialized = 0;

// Initialize buddy allocator once
static void initialize_buddy() {
    if (!buddy_initialized) {
        buddy_init(&global_buddy);
        buddy_initialized = 1;
    }
}

void* my_malloc(size_t size) {
    if (size == 0 || size > (2ULL * 1024 * 1024 * 1024)) return NULL;

    // Handle small allocations with buddy allocator
    if (size < SMALL_THRESHOLD) {
        initialize_buddy();
        return buddy_alloc(&global_buddy, global_buddy.min_block_size);
    }
    
    // Handle large allocations with mmap
    // Calculate size including metadata header
    size_t total_size = size + sizeof(size_t);
    // Round up to nearest page multiple
    size_t num_pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
    size_t alloc_size = num_pages * PAGE_SIZE;
    
    // Allocate memory with mmap
    void* base = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, 
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) return NULL;
    
    // Store allocation size in metadata header
    *((size_t*)base) = alloc_size;
    
    // Return pointer after metadata header
    return (char*)base + sizeof(size_t);
}

void my_free(void* ptr) {
    if (ptr == NULL) return;
    
    uintptr_t buddy_start = (uintptr_t)global_buddy.memory_pool;
    uintptr_t buddy_end = buddy_start + (1024 * 1024);
    uintptr_t current_ptr = (uintptr_t)ptr;
    
    // Handle buddy allocations
    if (current_ptr >= buddy_start && current_ptr < buddy_end) {
        buddy_free(&global_buddy, ptr);
        return;
    }
    
    // Handle mmap allocations
    // Retrieve metadata header
    void* base = (void*)(current_ptr - sizeof(size_t));
    size_t alloc_size = *((size_t*)base);
    
    // Unmap memory
    munmap(base, alloc_size);
}