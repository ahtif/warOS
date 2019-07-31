#include <stdio.h>
#include <kernel/system.h>
#include <kernel/tty.h>

void kernel_early_main(void) {
  gdt_install();
  idt_install();
  return;
}

void kernel_main(void) {
	terminal_initialize();
	printf("Hello, kernel World!\n");
  char c = '0';
  for (int i = 0; i < 25; i++) {
      printf("%c\n",c);
			if (++c >= 'z')
				c = '0';
  }
	printf("hi ");
	printf("my name is ahtif\n");
}
