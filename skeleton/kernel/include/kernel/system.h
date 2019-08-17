#ifndef _KERNEL_SYSTEM_H
#define _KERNEL_SYSTEM_H
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

#endif
