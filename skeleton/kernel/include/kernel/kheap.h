#ifndef _KERNEL_KHEAP_H
#define _KERNEL_KHEAP_H

#include <stdbool.h>
#include <stdint.h>

/**
   Allocate a chunk of memory, size in size. If align == 1,
   the chunk must be page-aligned. If phys != 0, the physical
   location of the allocated chunk will be stored into phys.

   This is the internal version of kmalloc. More user-friendly
   parameter representations are available in kmalloc, kmalloc_a,
   kmalloc_ap, kmalloc_p.
**/
void* kmalloc_int(uint32_t size, bool align, uint32_t *phys);

/**
   Allocate a chunk of memory, size in size. The chunk must be
   page aligned.
**/
void* kmalloc_a(uint32_t size);

/**
   Allocate a chunk of memory, size in size. The physical address
   is returned in phys. Phys MUST be a valid pointer to void*!
**/
void* kmalloc_p(uint32_t size, uint32_t *phys);

/**
   Allocate a chunk of memory, size in size. The physical address
   is returned in phys. It must be page-aligned.
**/
void* kmalloc_ap(uint32_t size, uint32_t *phys);

/**
   General allocation function.
**/
void* kmalloc(uint32_t size);

#endif // KHEAP_H
