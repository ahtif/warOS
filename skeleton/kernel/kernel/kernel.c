#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <kernel/system.h>
#include <kernel/tty.h>
#include <kernel/multiboot.h>

extern uint32_t *_kernel_phys_start, *_kernel_phys_end;
extern uint32_t	*_kernel_virt_start, *_kernel_virt_end;
extern uint32_t *stack_top, *stack_bottom;

void kernel_early_main(multiboot_info_t* mb_info, uint32_t magic) {
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
		abort();
	uint32_t stack_size = &stack_top - &stack_bottom;
	uint32_t mem_size = mb_info->mem_upper + mb_info->mem_lower;
	terminal_initialize();
	printf("stack top: %x\nstack bottom: %x\n", &stack_top, &stack_bottom);
	printf("stack size: %d\n", stack_size);
	printf("mem size: %d\n", mem_size);
  gdt_install();
  idt_install();
	//Setup paging


  isrs_install();
	irq_install();
	/*timer_install();
	keyboard_install();
	__asm__ __volatile__ ("sti"); */
  return;
}

void kernel_main(void) {
	//terminal_initialize();
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
  // putchar(d/0);
	for(;;);
}
