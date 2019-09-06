#include <stdio.h>
#include <arch/i386/system.h>

void panic(const char* msg, const char* file, uint32_t line)
{
  // We encountered a massive problem and have to stop.
  asm volatile("cli"); // Disable interrupts.

  printf("PANIC (%s) at %s:%d\n", msg, file, line);
  // Halt by going into an infinite loop.
  for(;;);
}
void panic_assert(const char* file, uint32_t line, const char* msg)
{
  // We encountered a massive problem and have to stop.
  asm volatile("cli"); // Disable interrupts.

  printf("ASSERTION FAILED (%s) at %s:%d\n", msg, file, line);
  // Halt by going into an infinite loop.
  for(;;);
}
