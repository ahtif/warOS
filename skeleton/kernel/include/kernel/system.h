#ifndef _KERNEL_SYSTEM_H
#define _KERNEL_SYSTEM_H

#define KERNEL_ADDR_OFFSET 0xC0000000
#define KERNEL_PLACEMENT_SIZE 0x400000
#define KERNEL_HEAP_SIZE 0x2800000
#define VIRTUAL_TO_PHYSICAL(addr) ((addr) - KERNEL_ADDR_OFFSET)
#define PHYSICAL_TO_VIRTUAL(addr) ((addr) + KERNEL_ADDR_OFFSET)

#include <stdint.h>

/* This defines what the stack looks like after an ISR was running */
struct regs
{
    unsigned int gs, fs, es, ds;      /* pushed the segs last */
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
    unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
    unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */
};

void gdt_set_gate(int, unsigned long, unsigned long, unsigned char, unsigned char);
void gdt_install();
void idt_set_gate(unsigned char, unsigned long, unsigned short, unsigned char);
void idt_install();
void isrs_install();
void irq_install();
void irq_install_handler(int irq, void (*handler)(struct regs *r));
void timer_install();
void keyboard_install();

void panic(const char*, const char*, uint32_t);
void panic_assert(const char*, uint32_t, const char*);

#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(cond) ((cond) ? (void)0 : panic_assert(__FILE__, __LINE__, #cond));


#endif
