#include <stdint.h>
#include <stddef.h>
#include <arch/i386/paging.h>
#include <kernel/kheap.h>
#include <kernel/heap.h>

// Addresses are defined in paging.c
extern uint32_t placement_start, placement_address, placement_end;
extern heap_t *kernel_heap;

void* kmalloc_int(uint32_t size, bool align, uint32_t *phys)
{
    if (kernel_heap)
      return heap_alloc(size, kernel_heap);
    // This will eventually call malloc() on the kernel heap.
    // For now, though, we just assign memory at placement_address
    // and increment it by size. Even when we've coded our kernel
    // heap, this will be useful for use before the heap is initialised.
    if (align && !IS_PAGE_ALIGNED(placement_address))
    {
        // Align the placement address;
        placement_address = ALIGN_UP(placement_address);
    }
    if (placement_address >= placement_end || placement_address + size >= placement_end)
      return NULL;
    if (phys)
    {
        *phys = placement_address;
    }
    void* tmp = (void*) placement_address;
    placement_address += size;
    return tmp;
}

void* kmalloc_a(uint32_t size)
{
    return kmalloc_int(size, true, 0);
}

void* kmalloc_p(uint32_t size, uint32_t *phys)
{
    return kmalloc_int(size, false, phys);
}

void* kmalloc_ap(uint32_t size, uint32_t *phys)
{
    return kmalloc_int(size, true, phys);
}

void* kmalloc(uint32_t size)
{
    return kmalloc_int(size, false, 0);
}

void kfree(void *ptr)
{
    heap_free(ptr, kernel_heap);
}
