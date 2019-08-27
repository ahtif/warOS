#include <stdint.h>
#include <stdbool.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>

// end is defined in the linker script.
extern uint32_t _kernel_virt_end;
uint32_t placement_address = (uint32_t)&_kernel_virt_end;

void* kmalloc_int(uint32_t size, bool align, uint32_t *phys)
{
    // This will eventually call malloc() on the kernel heap.
    // For now, though, we just assign memory at placement_address
    // and increment it by size. Even when we've coded our kernel
    // heap, this will be useful for use before the heap is initialised.
    if (align && !IS_PAGE_ALIGNED(placement_address))
    {
        // Align the placement address;
        placement_address = ALIGN_UP(placement_address);
    }
    if (phys)
    {
        *phys = placement_address;
    }
    void* tmp = placement_address;
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
