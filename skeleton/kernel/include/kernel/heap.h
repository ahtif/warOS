#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <stdint.h>

#define HEAP_MAGIC 0x123890AB

/**
 * A header associated with a heap entry
 */
typedef struct {
    size_t size;
    uint32_t magic;
    bool is_hole;
} heap_header_t;

/**
 * A footer associated with a heap entry
 */
typedef struct {
    uint32_t magic;
    heap_header_t* header;
} heap_footer_t;

/**
 * A heap index entry that corresponds to some headers in memory
 */
typedef struct {
    heap_header_t** entries;
    size_t size, max_size;
} heap_index_t;

/**
 * The parent heap structure
 */
typedef struct {
    uint32_t start_addr, end_addr;
    size_t occupied, capacity;
    heap_index_t* hole_index;
    bool is_kernel, is_writable;
} heap_t;

#define HEAP_OVERHEAD(num_entries) (sizeof(heap_t) + sizeof(heap_index_t) + sizeof(heap_header_t*)) * num_entries

/**
 * Create a new heap structure between a range of addresses. Make sure these addresses don't overlap
 * with another heap
 */
heap_t* heap_create(uint32_t start_addr, uint32_t end_addr, bool is_kernel, bool is_writable, uint32_t index_size);

/**
 * Allocate some memory on the heap
 */
void* heap_alloc(size_t size, heap_t* heap);

/**
 * Free some memory in the heap so that it can be reallocated
 */
void heap_free(void* ptr, heap_t* heap);

#endif //_KERNEL_HEAP_H
