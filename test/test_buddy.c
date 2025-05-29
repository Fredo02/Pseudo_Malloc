#include "buddy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test 1: Basic allocation and free
void test_basic_allocation() {
    BuddyAllocator buddy;
    buddy_init(&buddy);
    
    void* block = buddy_alloc(&buddy, 1024);
    assert(block != NULL);
    assert((uintptr_t)block % 1024 == 0);  // Check alignment
    
    buddy_free(&buddy, block);
    
    // Verify free worked
    void* block2 = buddy_alloc(&buddy, 1024);
    assert(block2 == block);
    
    buddy_free(&buddy, block2);
    printf("Test 1 (Basic Allocation) Passed\n");
}

// Test 2: Multiple allocations and reuse
void test_multiple_allocations() {
    BuddyAllocator buddy;
    buddy_init(&buddy);
    
    void* blocks[10];
    for (int i = 0; i < 10; i++) {
        blocks[i] = buddy_alloc(&buddy, 1024);
        assert(blocks[i] != NULL);
    }
    
    // Verify all blocks are distinct
    for (int i = 0; i < 10; i++) {
        for (int j = i + 1; j < 10; j++) {
            assert(blocks[i] != blocks[j]);
        }
    }
    
    // Free some blocks and reuse
    buddy_free(&buddy, blocks[2]);
    buddy_free(&buddy, blocks[5]);
    buddy_free(&buddy, blocks[7]);
    
    void* reused1 = buddy_alloc(&buddy, 1024);
    void* reused2 = buddy_alloc(&buddy, 1024);
    void* reused3 = buddy_alloc(&buddy, 1024);
    
    assert(reused1 == blocks[2] || reused1 == blocks[5] || reused1 == blocks[7]);
    assert(reused2 == blocks[2] || reused2 == blocks[5] || reused2 == blocks[7]);
    assert(reused3 == blocks[2] || reused3 == blocks[5] || reused3 == blocks[7]);
    
    // Cleanup
    for (int i = 0; i < 10; i++) {
        if (i != 2 && i != 5 && i != 7) {
            buddy_free(&buddy, blocks[i]);
        }
    }
    buddy_free(&buddy, reused1);
    buddy_free(&buddy, reused2);
    buddy_free(&buddy, reused3);
    
    printf("Test 2 (Multiple Allocations) Passed\n");
}

// Test 3: Splitting and merging
void test_splitting_and_merging() {
    BuddyAllocator buddy;
    buddy_init(&buddy);
    
    // Allocate and split a large block
    void* large_block = buddy_alloc(&buddy, 32 * 1024);  // 32KB
    assert(large_block != NULL);
    
    // Free should merge back to original block
    buddy_free(&buddy, large_block);
    
    // Allocate full 64KB block (requires merging)
    void* merged_block = buddy_alloc(&buddy, 64 * 1024);
    assert(merged_block != NULL);
    
    // Free the merged block
    buddy_free(&buddy, merged_block);
    
    // Allocate 1MB block (should be at base address)
    void* full_block = buddy_alloc(&buddy, 1024 * 1024);
    assert(full_block == buddy.memory_pool);
    
    buddy_free(&buddy, full_block);
    printf("Test 3 (Splitting and Merging) Passed\n");
}

// Test 4: Full allocation and OOM
void test_full_allocation() {
    BuddyAllocator buddy;
    buddy_init(&buddy);
    
    // Allocate entire memory as 1MB block
    void* full_block = buddy_alloc(&buddy, 1024 * 1024);
    assert(full_block != NULL);
    
    // Should be out of memory
    void* oom_block = buddy_alloc(&buddy, 1024);
    assert(oom_block == NULL);
    
    // Free and verify reuse
    buddy_free(&buddy, full_block);
    void* reused_block = buddy_alloc(&buddy, 1024 * 1024);
    assert(reused_block != NULL);
    
    buddy_free(&buddy, reused_block);
    printf("Test 4 (Full Allocation) Passed\n");
}

// Test 5: Edge cases and error handling
void test_edge_cases() {
    BuddyAllocator buddy;
    buddy_init(&buddy);
    
    // Allocate with size 0
    void* null_block = buddy_alloc(&buddy, 0);
    assert(null_block == NULL);
    
    // Allocate too large
    void* huge_block = buddy_alloc(&buddy, 2 * 1024 * 1024);
    assert(huge_block == NULL);
    
    // Invalid frees (should not crash)
    buddy_free(&buddy, NULL);
    buddy_free(&buddy, (void*)0xdeadbeef);
    buddy_free(&buddy, buddy.memory_pool + 512);  // Middle of block
    
    // Size rounding
    void* small_block = buddy_alloc(&buddy, 1);
    assert(small_block != NULL);
    buddy_free(&buddy, small_block);
    
    printf("Test 5 (Edge Cases) Passed\n");
}

// Test 6: Comprehensive free and reuse
void test_comprehensive_free() {
    BuddyAllocator buddy;
    buddy_init(&buddy);
    
    void* blocks[20];
    int sizes[] = {1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288};
    
    // Allocate various sizes
    for (int i = 0; i < 10; i++) {
        blocks[i] = buddy_alloc(&buddy, sizes[i]);
        assert(blocks[i] != NULL);
    }
    
    // Free all blocks
    for (int i = 0; i < 10; i++) {
        buddy_free(&buddy, blocks[i]);
    }
    
    // Allocate full memory
    void* full_block = buddy_alloc(&buddy, 1024 * 1024);
    assert(full_block != NULL);
    
    // Free and verify we can allocate all sizes again
    buddy_free(&buddy, full_block);
    for (int i = 0; i < 10; i++) {
        blocks[i] = buddy_alloc(&buddy, sizes[i]);
        assert(blocks[i] != NULL);
    }
    
    // Final cleanup
    for (int i = 0; i < 10; i++) {
        buddy_free(&buddy, blocks[i]);
    }
    
    printf("Test 6 (Comprehensive Free) Passed\n");
}

int main() {
    test_basic_allocation();
    test_multiple_allocations();
    test_splitting_and_merging();
    test_full_allocation();
    test_edge_cases();
    test_comprehensive_free();
    
    printf("All tests passed successfully!\n");
    return 0;
}