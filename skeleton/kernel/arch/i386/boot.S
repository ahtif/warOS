# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

.set KERNEL_ADDR_OFFSET, 0xC0000000
.set KERNEL_PAGE_NUMBER, KERNEL_ADDR_OFFSET >> 22
# One for kernel code, one for temporary allocation space and 5 for the heap. Each covers 4MiB
.set KERNEL_NUM_UPPER_PAGES, 7

# Declare a header as in the Multiboot Standard.
.section .multiboot.data, "a"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM


.align 4096
# The initial page directory used to boot into the higher half
# Allocates seven 4MB pages for kernel, one for the initial 4MB, and six in the upper 4MB for the kernel code, kernel pile and
# kernel heap
boot_page_directory:
    # Set lower half pages. This will be unmapped later on as we'll be jumping out of the lower 4MiB
    .long 0x00000083
    # Fill all preceding pages with 4 bytes
    .fill (KERNEL_PAGE_NUMBER - 1), 4
    # Set higher half pages, starting from 0 and increasing by 4MiB
    .long 0x00000083
    .long 0x00000083 | (1 << 22)
    .long 0x00000083 | (2 << 22)
    .long 0x00000083 | (3 << 22)
    .long 0x00000083 | (4 << 22)
    .long 0x00000083 | (5 << 22)
    .long 0x00000083 | (6 << 22)
     # Fill all succeeding pages with 4 bytes
    .fill (1024 - KERNEL_PAGE_NUMBER - KERNEL_NUM_UPPER_PAGES), 4

# The kernel entry point.
.global _start
.type _start, @function
_start:

	# Load in the page directory
	mov $boot_page_directory, %ecx
	mov %ecx, %cr3

	mov %cr4, %ecx
	# Set PSE bit in to enable 4MB pages
	or $0x00000010, %ecx
	mov %ecx, %cr4

	# Enable paging
	mov %cr0, %ecx
	or $0x80000000, %ecx
	mov %ecx, %cr0
	jmp start_higher_half


.section .text
# This will set up our new segment registers. We need to do
# something special in order to set CS. We do what is called a
# far jump. A jump that includes a segment as well as an offset.
# This is declared in C as 'extern void gdt_flush()#'
.global gdt_flush     # Allows the C code to link to this
.extern gp            # Says that '_gp' is in another file
gdt_flush:
	lgdt gp        # Load the GDT with our '_gp' which is a special pointer
	mov $0x10, %ax      # 0x10 is the offset in the GDT to our data segment
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss

	# Enter protected mode
	mov %cr0, %eax
	or $0x1, %al
	mov %eax, %cr0

	ljmp $0x08, $reload_cs   # 0x08 is the offset to our code segment: Far jump!
reload_cs:
  ret               # Returns back to the C code!


# Loads the IDT defined in '_idtp' into the processor.
# This is declared in C as 'extern void idt_load();'
.global idt_load
.extern idtp
idt_load:
  lidt idtp
  ret


# At this point, paging is fully set up and enabled.
start_higher_half:

	# Unmap the identity mapping as it is now unnecessary.
	invlpg (0)

  # Set up the stack
	movl $stack_top, %esp

 	#Push the magic value.
  push %eax
	# Push multiboot header address. with the virtual offset
  add $KERNEL_ADDR_OFFSET, %ebx
	push %ebx
	# call early main
	call kernel_early_main

	# Call the global constructors.
	call _init

	#movl $_kernel_start, %esi
	#movl $_kernel_end, %edi
	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b

.section .bss
.align 16
.global stack_bottom
.global stack_top
stack_bottom:
.fill 16384 # 16 KiB
stack_top:

# In just a few pages in this tutorial, we will add our Interrupt
# Service Routines (ISRs) right here!
.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31

#  0: Divide By Zero Exception
isr0:
    cli
    push $0   # A normal ISR stub that pops a dummy error code to keep a
               # uniform stack frame
    push $0
    jmp isr_common_stub

#  1: Debug Exception
isr1:
    cli
    push $0
    push $1
    jmp isr_common_stub

#  2: Non Maskable Interrupt Exception
isr2:
    cli
    push $0
    push $2
    jmp isr_common_stub

#  3: Breakpoint Exception
isr3:
    cli
    push $0
    push $3
    jmp isr_common_stub

#  4: Into Detected Overflow Exception
isr4:
    cli
    push $0
    push $4
    jmp isr_common_stub

#  5: Out of Bounds Exception
isr5:
    cli
    push $0
    push $5
    jmp isr_common_stub

#  6: Invalid Opcode Exception
isr6:
    cli
    push $0
    push $6
    jmp isr_common_stub

#  7: No Coprocessor Exception
isr7:
    cli
    push $0
    push $7
    jmp isr_common_stub

#  8: Double Fault Exception (With Error Code!)
isr8:
    cli
    push $8        # Note that we DON'T push a value on the stack in this one!
                   # It pushes one already! Use this type of stub for exceptions
                   # that pop error codes!
    jmp isr_common_stub

#  9: Coprocessor Segment Overrun Exception
isr9:
    cli
    push $0
    push $9
    jmp isr_common_stub

#  10: Bad TSS Exception
isr10:
    cli
    push $10
    jmp isr_common_stub

#  11: Segnment not Present Exception
isr11:
    cli
    push $11
    jmp isr_common_stub

#  12: Stack Fault Exception
isr12:
    cli
    push $12
    jmp isr_common_stub

#  13: General Protection Fault Exception
isr13:
    cli
    push $13
    jmp isr_common_stub

#  14: Page Fault Exception
isr14:
    cli
    push $14
    jmp isr_common_stub

#  15: Unknown Interrupt Exception
isr15:
    cli
    push $0
    push $15
    jmp isr_common_stub

#  16: Coprocessor Fault Exception
isr16:
    cli
    push $0
    push $16
    jmp isr_common_stub

#  17: Alignment Check Exception (486+)
isr17:
    cli
    push $17
    jmp isr_common_stub

#  18: Machine Check Exception (Pentium/586+)
isr18:
    cli
    push $0
    push $18
    jmp isr_common_stub

#  19 to 29: Reserved Exceptions
isr19:
    cli
    push $0
    push $19
    jmp isr_common_stub

isr20:
    cli
    push $0
    push $20
    jmp isr_common_stub

isr21:
    cli
    push $0
    push $21
    jmp isr_common_stub

isr22:
    cli
    push $0
    push $22
    jmp isr_common_stub

isr23:
    cli
    push $0
    push $23
    jmp isr_common_stub

isr24:
    cli
    push $0
    push $24
    jmp isr_common_stub

isr25:
    cli
    push $0
    push $25
    jmp isr_common_stub

isr26:
    cli
    push $0
    push $26
    jmp isr_common_stub

isr27:
    cli
    push $0
    push $27
    jmp isr_common_stub

isr28:
    cli
    push $0
    push $28
    jmp isr_common_stub

isr29:
    cli
    push $0
    push $29
    jmp isr_common_stub

#  30: Security Exception
isr30:
    cli
    push $30
    jmp isr_common_stub

#  31: Reserved Exception
isr31:
    cli
    push $0
    push $31
    jmp isr_common_stub

# We call a C function in here. We need to let the assembler know
# that '_fault_handler' exists in another file
.extern fault_handler

# This is our common ISR stub. It saves the processor state, sets
# up for kernel mode segments, calls the C-level fault handler,
# and finally restores the stack frame.
isr_common_stub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax   # Load the Kernel Data Segment descriptor!
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %esp, %eax  # Push us the stack
    push %eax
    mov $fault_handler, %eax
    call %eax       # A special call, preserves the 'eip' register
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp    # Cleans up the pushed error code and pushed ISR number
    iret           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15

# 32: IRQ0
irq0:
    cli
    push $0   # A normal ISR stub that pops a dummy error code to keep a
               # uniform stack frame
    push $32
    jmp irq_common_stub

# 33: IRQ1
irq1:
    cli
    push $0
    push $33
    jmp irq_common_stub

# 34: IRQ2
irq2:
    cli
    push $0
    push $34
    jmp irq_common_stub

# 35: IRQ3
irq3:
    cli
    push $0
    push $35
    jmp irq_common_stub

# 36: IRQ4
irq4:
    cli
    push $0
    push $36
    jmp irq_common_stub

# 37: IRQ5
irq5:
    cli
    push $0
    push $37
    jmp irq_common_stub

# 38: IRQ6
irq6:
    cli
    push $0
    push $38
    jmp irq_common_stub

# 39: IRQ7
irq7:
    cli
    push $0
    push $39
    jmp irq_common_stub

# 40: IRQ8
irq8:
    cli
    push $0
    push $40
    jmp irq_common_stub

# 41: IRQ9
irq9:
    cli
    push $0
    push $41
    jmp irq_common_stub

# 42: IRQ10
irq10:
    cli
		push $0
    push $42
    jmp irq_common_stub

# 43: IRQ11
irq11:
    cli
		push $0
    push $43
    jmp irq_common_stub


# 44: IRQ12
irq12:
    cli
		push $0
    push $44
    jmp irq_common_stub

# 45: IRQ13
irq13:
    cli
		push $0
    push $45
    jmp irq_common_stub

# 46: IRQ14
irq14:
    cli
		push $0
    push $46
    jmp irq_common_stub


# 47: IRQ15
irq15:
    cli
    push $0
    push $47
    jmp irq_common_stub

.extern irq_handler

# This is a stub that we have created for IRQ based ISRs. This calls
# '_irq_handler' in our C code. We need to create this in an 'irq.c'
irq_common_stub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax   # Load the Kernel Data Segment descriptor!
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %esp, %eax  # Push us the stack
    push %eax
    mov $irq_handler, %eax
    call %eax       # A special call, preserves the 'eip' register
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp    # Cleans up the pushed error code and pushed ISR number
    iret           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
