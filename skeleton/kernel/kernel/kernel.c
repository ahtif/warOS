#include <stdio.h>
#include <kernel/system.h>
#include <kernel/tty.h>

void kernel_early_main(void) {
	terminal_initialize();
  gdt_install();
  idt_install();
  isrs_install();
	irq_install();
	timer_install();
	keyboard_install();
	__asm__ __volatile__ ("sti");
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
	for(;;);
 //}  //putchar(d/0);
}
