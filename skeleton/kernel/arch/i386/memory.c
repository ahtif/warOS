#include <kernel/tty.h>
#include <stdio.h>
#include <kernel/multiboot.h>
#include <kernel/memory.h>

extern int _kernel_start;
extern int _kernel_end;

void memory_init()
{
    //putchar(_kernel_start);
    //putchar(_kernel_end);

    return;
}

void early_mem(unsigned long magic, unsigned long addr)
{

  return;
  // /* Am I booted by a Multiboot-compliant boot loader? */
  // if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
  // {
  //   printf ("Invalid magic number\n");
  //   return;
  // } else {
  //   printf("valid magic number\n");
  //   return;
  // }

}
