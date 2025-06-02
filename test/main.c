#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Declare the allocation functions from allocator.c
void* my_malloc(size_t size);
void my_free(void* ptr);

// Threshold between small and large allocations (1/4 page = 1024 bytes)
#define SMALL_THRESHOLD 1024
// Maximum number of allocations to track
#define MAX_ALLOCS 1000

int main() {
    void* pointers[MAX_ALLOCS]; // Array to store allocated pointers
    int count = 0;              // Current number of allocations
    size_t size;                // User input for allocation size
    char choice;                // User input for next action

    printf("Memory allocator interactive program\n");

    while (1) {
        // Prompt for allocation size
        printf("\nEnter allocation size in bytes: ");
        if (scanf("%zu", &size) != 1) {
            // Clear invalid input
            while (getchar() != '\n');
            printf("Invalid input. Please enter a positive integer.\n");
            continue;
        }

        // Allocate memory
        void* ptr = my_malloc(size);
        if (ptr == NULL) {
            printf("Failed to allocate %zu bytes.\n", size);
        } else {
            // Store pointer if allocation successful
            if (count < MAX_ALLOCS) {
                pointers[count++] = ptr;
                // Print allocation method based on size
                if (size < SMALL_THRESHOLD) {
                    printf("%zu bytes allocated using buddy\n", size);
                } else {
                    printf("%zu bytes allocated using mmap\n", size);
                }
            } else {
                printf("Maximum allocations reached (%d). Freeing this block.\n", MAX_ALLOCS);
                my_free(ptr);
            }
        }

        // Prompt for next action
        printf("Do you want to (a) make another allocation or (b) free all blocks and exit? [a/b]: ");
        scanf(" %c", &choice); // Space to skip whitespace

        if (choice == 'b' || choice == 'B') {
            // Free all allocated blocks and exit
            for (int i = 0; i < count; i++) {
                my_free(pointers[i]);
            }
            printf("All blocks freed. Exiting program.\n");
            break;  // Exit the program after freeing
        } else if (choice != 'a' && choice != 'A') {
            printf("Invalid choice. Continuing with allocations.\n");
        }
    }

    return 0;
}