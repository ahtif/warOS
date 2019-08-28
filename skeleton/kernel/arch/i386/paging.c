#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <kernel/system.h>
#include <kernel/paging.h>
#include <kernel/kheap.h>
#include <kernel/heap.h>

#define FRAMES_PER_BITMAP 32
#define HEAP_INDEX_SIZE 0x20000

// The kernel's page directory
page_directory_t *kernel_directory=NULL;

// The current page directory;
page_directory_t *current_directory=NULL;
heap_t *kernel_heap = NULL;
uint32_t placement_start, placement_address, placement_end;

// A bitset of frames - used or free.
uint32_t *frames;
uint32_t nframes;

// Defined in kheap.c
extern uint32_t placement_address;

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a / FRAMES_PER_BITMAP)
#define OFFSET_FROM_BIT(a) (a % FRAMES_PER_BITMAP)

// Static function to set a bit in the frames bitset
static void set_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/PAGE_SIZE;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/PAGE_SIZE;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
static uint32_t test_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr/PAGE_SIZE;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
static uint32_t first_frame()
{
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
    {
        if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
        {
            // at least one bit is free here.
            for (j = 0; j < 32; j++)
            {
                uint32_t toTest = 0x1 << j;
                if ( !(frames[i]&toTest) )
                {
                    return i*4*8+j;
                }
            }
        }
    }
    return -1;
}

// Function to allocate a frame.
void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
    if (page->frame != 0)
    {
        return;
    }
    else
    {
        uint32_t idx = first_frame();
        if (idx == (uint32_t)-1)
        {
            // PANIC! no free frames!!
            PANIC("No free frames!")
        }
        set_frame(idx*PAGE_SIZE);
        page->present = 1;
        page->rw = (is_writeable)?1:0;
        page->user = (is_kernel)?0:1;
        page->frame = idx;
    }
}

// Function to deallocate a frame.
void free_frame(page_t *page)
{
    uint32_t frame;
    if (!(frame=page->frame))
    {
        return;
    }
    else
    {
        clear_frame(frame);
        page->frame = 0x0;
    }
}


void init_paging(uint32_t mem_size, uint32_t phys_start, uint32_t phys_end,
  uint32_t virt_start, uint32_t virt_end)
{
    // The size of physical memory. Convert to bytes
    uint32_t aligned_mem = ALIGN_UP(mem_size * 1024);

    placement_start = virt_end;
    placement_address = placement_start;
    placement_end = placement_start + KERNEL_PLACEMENT_SIZE;
    uint32_t heap_start = placement_end;
    uint32_t heap_end = heap_start + KERNEL_HEAP_SIZE;

    uint32_t map_pstart = ALIGN_DOWN(phys_start);
    uint32_t map_pend = ALIGN_UP(phys_end);
    uint32_t map_vstart = ALIGN_DOWN(virt_start);
    uint32_t map_vend = ALIGN_UP(virt_end);



    // Let's make a page directory.
     kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
    current_directory = kernel_directory;
    map_directory(kernel_directory, map_pstart, map_pend, map_vstart, map_vend);
    printf("address of dir: %x\n", (uint32_t)kernel_directory);

    nframes = aligned_mem / PAGE_SIZE;
    frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes));
    memset(frames, 0, INDEX_FROM_BIT(nframes));
    // We need to identity map (phys addr = virt addr) from
    // 0x0 to the end of used memory, so we can access this
    // transparently, as if paging wasn't enabled.
    // NOTE that we use a while loop here deliberately.
    // inside the loop body we actually change placement_address
    // by calling kmalloc(). A while loop causes this to be
    // computed on-the-fly rather than once at the start.
    uint32_t start = ALIGN_DOWN(map_pstart);
    uint32_t end = ALIGN_DOWN(map_pend);

    for (uint32_t frame = start; frame < end; frame += PAGE_SIZE)
    {
      set_frame(frame);
    }
    // while (i < placement_address)
    // {
    //     // Kernel code is readable but not writeable from userspace.
    //     //alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
    //     i += PAGE_SIZE;
    // }

    // Now, enable paging!
    //switch_page_directory(kernel_directory);
    // Before we enable paging, we must register our page fault handler.
    irq_install_handler(14, page_fault);
    switch_page_directory(VIRTUAL_TO_PHYSICAL((uint32_t)kernel_directory));
    // Create the kernel heap
    kernel_heap = heap_create(heap_start, heap_end, true, true, HEAP_INDEX_SIZE);

}

void switch_page_directory(page_directory_t *dir)
{
    current_directory = dir;
    asm volatile("mov %0, %%cr3":: "r"(dir));
    /*uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));*/
}

// page_t *get_page(uint32_t address, int make, page_directory_t *dir)
// {
//     // Turn the address into an index.
//     address /= PAGE_SIZE;
//     // Find the page table containing this address.
//     uint32_t table_idx = address / 1024;
//     if (dir->tables[table_idx]) // If this table is already assigned
//     {
//         return &dir->tables[table_idx]->pages[address%1024];
//     }
//     else if(make)
//     {
//         uint32_t tmp;
//         dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
//         dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
//         return &dir->tables[table_idx]->pages[address%1024];
//     }
//     else
//     {
//         return 0;
//     }
// }


void map_page(page_directory_t* dir, uint32_t page, uint32_t phys_addr) {
    page_dir_entry_t* entry = &dir->entries[page];
    entry->accessed = 0;
    entry->cache_off = 0;
    entry->page_size = 1;
    entry->unused = 0;
    entry->present = 1;
    entry->access_lvl = 0;
    entry->rw_flag = 1;
    entry->write_through = 1;
    // printf("%x mod %x == %d\n", PAGE_SIZE, phys_addr, (PAGE_SIZE % phys_addr));
    // ASSERT((PAGE_SIZE % phys_addr) == 0)
    // Shift to isolate the top 10 bits (the 4MiB aligned physical address)
    entry->tbl_addr = phys_addr >> 22;
}

void map_directory(page_directory_t* dir, uint32_t phys_start, uint32_t phys_end, uint32_t virt_start, uint32_t virt_end) {
    if (phys_start > phys_end) PANIC("Improper start and end physical addresses");
    if(!IS_PAGE_ALIGNED(phys_start) || !IS_PAGE_ALIGNED(phys_end)) PANIC("Physical addresses are not page aligned");
    if (virt_start > virt_end) PANIC("Improper start and end virtual addresses");
    if(!IS_PAGE_ALIGNED(virt_start) || !IS_PAGE_ALIGNED(virt_end)) PANIC("Virtual addresses are not page aligned");
    if(virt_end - virt_start != phys_end - phys_start) PANIC("Virtual space is not equal to physical space");

    if (dir) {
        memset(dir, 0, sizeof(page_directory_t));
        uint32_t virt_addr = virt_start, phys_addr = phys_start;
        uint32_t page = virt_addr / PAGE_SIZE;
        while (page < NUM_ENTRIES && virt_addr <= virt_end) {
            map_page(dir, page, phys_addr);
            phys_addr += PAGE_SIZE;
            virt_addr += PAGE_SIZE;
            page++;
        }
    }
}

void page_fault(struct regs *r)
{
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register->
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    // The error code gives us details of what happened.
    int present   = !(r->err_code & 0x1); // Page not present
    int rw = r->err_code & 0x2;           // Write operation?
    int us = r->err_code & 0x4;           // Processor was in user-mode?
    int reserved = r->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int id = r->err_code & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
    printf("Page fault! ( ");
    if (present) {printf("present ");}
    if (rw) {printf("read-only ");}
    if (us) {printf("user-mode ");}
    if (reserved) {printf("reserved ");}
    printf(") at 0x%x\n", faulting_address);
    PANIC("Page fault");
}
