This is a malloc replacement
   The system relies on mmap for the physical allocation of memory, but handles the requests in
   2 ways:
   - for small requests (< 1/4 of the page size) it uses a buddy allocator.
     Clearly, such a buddy allocator can manage at most page-size bytes
     For simplicity use a single buddy allocator, implemented with a bitmap
     that manages 1 MB of memory for these small allocations.

   - for large request (>=1/4 of the page size) uses a mmap.


How it works:
  1. Requesting allocation size:
      The program prompts the user to enter the size (in bytes) of the memory to allocate.

  2. Memory allocation:
      If the requested size is less than 1024 bytes, memory is allocated using the buddy allocator.
      If the size is 1024 bytes or more, memory is allocated using mmap.
      The program informs the user about the allocation method used.

  3. Pointer management:
      Each pointer returned by my_malloc is stored in an array, up to a maximum of 1000 simultaneous allocations.

  4. Error handling:
    If allocation fails, an error message is displayed.
    If the maximum number of allocations is exceeded, the newly allocated block is immediately freed.

  5. User choice:
    After each allocation, the user can choose to:
      Make another allocation (a)
      Free all allocated blocks and exit the program (b)

  6. Deallocation:
    If the user chooses to exit, the program frees all previously allocated blocks and prints an exit message.