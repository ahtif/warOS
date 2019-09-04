#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <kernel/system.h>
#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/paging.h>

extern void *_kernel_offset;
extern void *_kernel_phys_start, *_kernel_phys_end;
extern void	*_kernel_virt_start, *_kernel_virt_end;
extern uint32_t *stack_top, *stack_bottom;

void kernel_early_main(multiboot_info_t* mb_info, uint32_t magic) {
	terminal_initialize();
	ASSERT(magic == MULTIBOOT_BOOTLOADER_MAGIC)
	uint32_t mem_size = mb_info->mem_upper + mb_info->mem_lower;
	uint32_t stack_size = &stack_top - &stack_bottom;
	printf("phys_start: %x, phys_end: %x\nvirt_start: %x, virt_end: %x\n",
	&_kernel_phys_start, &_kernel_phys_end, &_kernel_virt_start, &_kernel_virt_end);
	printf("_kernel_offset: %x, offset in dec: %d\n", &_kernel_offset, &_kernel_offset);
	printf("stack top: %x\nstack bottom: %x\n", &stack_top, &stack_bottom);
	printf("stack size: %d\n", stack_size);
	printf("mem size: %d\n", mem_size*1024);
  gdt_install();
  idt_install();
  isrs_install();
	irq_install();
  timer_install();
	keyboard_install();
	init_paging(mem_size, (uint32_t)&_kernel_phys_start, (uint32_t)&_kernel_phys_end,
	 (uint32_t)&_kernel_virt_start, (uint32_t)&_kernel_virt_end);
	__asm__ __volatile__ ("sti");	//Setup paging

  return;
}

void kernel_main(void) {
	//terminal_initialize();
	uint32_t stack_size = &stack_top - &stack_bottom;
  multitasking_init(&stack_bottom, stack_size);
	printf("Hello, kernel World!\n");
  char c = '0';
  for (int i = 0; i < 10; i++) {
      printf("%c\n",c);
			if (++c >= 'z')
				c = '0';
  }
	printf("hi ");
	printf("my name is ahtif\n");
  int d = 5;
	// uint32_t *ptr = (uint32_t*) 0x0;
	// uint32_t fault = *ptr;
  // putchar(d/0);
	for(;;);
}
