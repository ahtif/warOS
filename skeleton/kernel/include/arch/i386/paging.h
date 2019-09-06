#ifndef _ARCH_I386_PAGING_H
#define _ARCH_I386_PAGING_H

#include <arch/i386/system.h>

#define PAGE_SIZE_4MB 0x400000
#define PAGE_SIZE_4KB 0x1000
#define PAGE_SIZE PAGE_SIZE_4MB
#define NUM_ENTRIES 1024

#define ALIGN_DOWN(addr) ((addr) - ((addr) % PAGE_SIZE))
#define ALIGN_UP(addr) (ALIGN_DOWN(addr) + PAGE_SIZE)
#define IS_PAGE_ALIGNED(addr) ((addr % PAGE_SIZE) == 0)

typedef struct page_dir_entry
{
    unsigned int present       : 1;
    unsigned int rw_flag       : 1;
    unsigned int access_lvl    : 1; //0 is for only ring0. 1 is for anybody.
    unsigned int write_through : 1;
    unsigned int cache_off     : 1;
    unsigned int accessed      : 1;
    unsigned int zero          : 1;
    unsigned int page_size     : 1;
    unsigned int unused        : 3;
    unsigned int ignored       : 1;
    unsigned int reserved      : 10;
    unsigned int tbl_addr      : 10;
} page_dir_entry_t;

typedef struct page_table_entry
{
    unsigned int present       : 1;
    unsigned int rw_flag       : 1;
    unsigned int access_lvl    : 1; //0 is for only ring0. 1 is for anybody.
    unsigned int write_through : 1;
    unsigned int cache_off     : 1;
    unsigned int accessed      : 1;
    unsigned int dirty         : 1;
    unsigned int zero          : 1;
    unsigned int global        : 1;
    unsigned int reserved      : 3;
    unsigned int phys_addr     : 20;
} page_table_entry_t;


typedef struct page
{
   uint32_t present    : 1;   // Page present in memory
   uint32_t rw         : 1;   // Read-only if clear, readwrite if set
   uint32_t user       : 1;   // Supervisor level only if clear
   uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
   uint32_t dirty      : 1;   // Has the page been written to since last refresh?
   uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
   uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_directory
{
  page_dir_entry_t entries[NUM_ENTRIES];
} page_directory_t;

// typedef struct page_table
// {
//    page_t pages[1024];
// } page_table_t;
//
// typedef struct page_directory
// {
//    /*
//       Array of pointers to pagetables.
//    */
//    page_table_t *tables[1024];
//    /*
//       Array of pointers to the pagetables above, but gives their *physical*
//       location, for loading into the CR3 register.
//    */
//    uint32_t tablesPhysical[1024];
//    /*
//       The physical address of tablesPhysical. This comes into play
//       when we get our kernel heap allocated and the directory
//       may be in a different location in virtual memory.
//    */
//    uint32_t physicalAddr;
// } page_directory_t;

/*
  Sets up the environment, page directories etc and
  enables paging.
*/
void init_paging(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

/*
  Causes the specified page directory to be loaded into the
  CR3 register.
*/
void switch_page_directory(page_directory_t *new);

/*
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
*/
page_t *get_page(uint32_t address, int make, page_directory_t *dir);


void map_directory(page_directory_t* dir, uint32_t phys_start, uint32_t phys_end,
  uint32_t virt_start, uint32_t virt_end);

void map_page(page_directory_t* dir, uint32_t page, uint32_t address);

/*
  Handler for page faults.
*/
void page_fault(struct regs *r);

#endif
