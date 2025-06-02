#include "allocator.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define SMALL_THRESHOLD (PAGE_SIZE / 4)  // 1024 bytes
#define BUDDY_POOL_SIZE (1024 * 1024)    // 1MB

// Test 1: Basic small allocation
void test_basic_small_allocation() {
    printf("Test 1: Basic small allocation... ");
    void* ptr = my_malloc(SMALL_THRESHOLD - 1);
    assert(ptr != NULL && "Allocation failed");
    
    // Check alignment
    assert((uintptr_t)ptr % 1024 == 0 && "Small allocation not aligned");

    // Write and read data
    memset(ptr, 0xAB, SMALL_THRESHOLD - 1);
    for (int i = 0; i < SMALL_THRESHOLD - 1; i++) {
        assert(((char*)ptr)[i] == (char)0xAB && "Memory corruption");
    }
    
    my_free(ptr);
    printf("Passed\n");
}

// Test 2: Basic large allocation
void test_basic_large_allocation() {
    printf("Test 2: Basic large allocation... ");
    size_t large_size = 10 * PAGE_SIZE;
    void* ptr = my_malloc(large_size);
    assert(ptr != NULL && "Large allocation failed");
    
    // Write and read data
    memset(ptr, 0xCD, large_size);
    for (size_t i = 0; i < large_size; i++) {
        assert(((char*)ptr)[i] == (char)0xCD && "Memory corruption");
    }
    
    my_free(ptr);
    printf("Passed\n");
}

// Test 3: Multiple small allocations
void test_multiple_small_allocations() {
    printf("Test 3: Multiple small allocations... ");
    void* ptrs[100];
    size_t sizes[] = {16, 32, 64, 128, 256, 512, 1023};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    // Allocate multiple small blocks
    for (int i = 0; i < 100; i++) {
        size_t size = sizes[i % num_sizes];
        ptrs[i] = my_malloc(size);
        assert(ptrs[i] != NULL && "Allocation failed");
        memset(ptrs[i], i, size);
    }
    
    // Verify and free
    for (int i = 0; i < 100; i++) {
        size_t size = sizes[i % num_sizes];
        for (size_t j = 0; j < size; j++) {
            assert(((char*)ptrs[i])[j] == (char)i && "Memory corruption");
        }
        my_free(ptrs[i]);
    }
    printf("Passed\n");
}

// Test 4: Multiple large allocations
void test_multiple_large_allocations() {
    printf("Test 4: Multiple large allocations... ");
    void* ptrs[10];
    size_t sizes[] = {PAGE_SIZE, 2 * PAGE_SIZE, 4 * PAGE_SIZE, 8 * PAGE_SIZE};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    // Allocate large blocks
    for (int i = 0; i < 10; i++) {
        ptrs[i] = my_malloc(sizes[i % num_sizes]);
        assert(ptrs[i] != NULL && "Allocation failed");
        memset(ptrs[i], i, sizes[i % num_sizes]);
    }
    
    // Verify and free
    for (int i = 0; i < 10; i++) {
        size_t size = sizes[i % num_sizes];
        for (size_t j = 0; j < size; j++) {
            assert(((char*)ptrs[i])[j] == (char)i && "Memory corruption");
        }
        my_free(ptrs[i]);
    }
    printf("Passed\n");
}

// Test 5: Mixed allocation types
void test_mixed_allocations() {
    printf("Test 5: Mixed allocations... ");
    void* small1 = my_malloc(512);
    void* large1 = my_malloc(2 * PAGE_SIZE);
    void* small2 = my_malloc(768);
    void* large2 = my_malloc(3 * PAGE_SIZE);
    
    assert(small1 != NULL);
    assert(large1 != NULL);
    assert(small2 != NULL);
    assert(large2 != NULL);
    
    // Verify distinct addresses
    assert(small1 != small2);
    assert(large1 != large2);
    assert(small1 != large1);
    assert(small1 != large2);
    assert(small2 != large1);
    assert(small2 != large2);
    
    // Write data
    memset(small1, 0x11, 512);
    memset(large1, 0x22, 2 * PAGE_SIZE);
    memset(small2, 0x33, 768);
    memset(large2, 0x44, 3 * PAGE_SIZE);
    
    // Free in random order
    my_free(large1);
    my_free(small1);
    my_free(large2);
    my_free(small2);
    printf("Passed\n");
}

// Test 6: Edge cases and error handling
void test_edge_cases() {
    printf("Test 6: Edge cases... ");
    // Zero size
    void* null_ptr = my_malloc(0);
    assert(null_ptr == NULL);
    
    // Free NULL pointer
    my_free(NULL);
    
    // Double free protection
    void* ptr = my_malloc(128);
    assert(ptr != NULL);
    my_free(ptr);
    my_free(ptr);  // Should not crash
    
    // Test allocation after freeing everything
    void* final_ptr = my_malloc(1024);
    assert(final_ptr != NULL);
    my_free(final_ptr);
    
    printf("Passed\n");
}

int main() {
    test_basic_small_allocation();
    test_basic_large_allocation();
    test_multiple_small_allocations();
    test_multiple_large_allocations();
    test_mixed_allocations();
    test_edge_cases();
    
    printf("All allocator tests passed successfully!\n");
    return 0;
}