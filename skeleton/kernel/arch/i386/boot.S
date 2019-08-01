# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Allocate the initial stack.
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.

# The kernel entry point.
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


.global _start
.type _start, @function
_start:
  # Physical address of boot_page_table1.
	# TODO: I recall seeing some assembly that used a macro to do the
	#       conversions to and from physical. Maybe this should be done in this
	#       code as well?
	movl $(boot_page_table1 - 0xC0000000), %edi
	# First address to map is address 0.
	# TODO: Start at the first kernel page instead. Alternatively map the first
	#       1 MiB as it can be generally useful, and there's no need to
	#       specially map the VGA buffer.
	movl $0, %esi
	# Map 1023 pages. The 1024th will be the VGA text buffer.
	movl $1023, %ecx


1:
	# Only map the kernel.
	cmpl $(_kernel_start - 0xC0000000), %esi
	jl 2f
	cmpl $(_kernel_end - 0xC0000000), %esi
	jge 3f

	# Map physical address as "present, writable". Note that this maps
	# .text and .rodata as writable. Mind security and map them as non-writable.
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

2:
	# Size of page is 4096 bytes.
	addl $4096, %esi
	# Size of entries in boot_page_table1 is 4 bytes.
	addl $4, %edi
	# Loop to the next entry if we haven't finished.
	loop 1b

3:
	# Map VGA video memory to 0xC03FF000 as "present, writable".
	movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

	# The page table is used at both page directory entry 0 (virtually from 0x0
	# to 0x3FFFFF) (thus identity mapping the kernel) and page directory entry
	# 768 (virtually from 0xC0000000 to 0xC03FFFFF) (thus mapping it in the
	# higher half). The kernel is identity mapped because enabling paging does
	# not change the next instruction, which continues to be physical. The CPU
	# would instead page fault if there was no identity mapping.

	# Map the page table to both virtual addresses 0x00000000 and 0xC0000000.
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

	# Set cr3 to the address of the boot_page_directory.
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	# Enable paging and the write-protect bit.
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	# Jump to higher half with an absolute jump.
	lea 4f, %ecx
	jmp *%ecx

4:
	# At this point, paging is fully set up and enabled.

	# Unmap the identity mapping as it is now unnecessary.
	movl $0, boot_page_directory + 0

	# Reload crc3 to force a TLB flush so the changes to take effect.
	movl %cr3, %ecx
	movl %ecx, %cr3

	# Set up the stack
	movl $stack_top, %esp

	# call early main
	call kernel_early_main

	# Call the global constructors.
	call _init




	# Set up GDT
# gdtr DW 0 # For limit storage
#      DD 0 # For base storage
#
# setGdt:
#    MOV   EAX, [esp + 4]
#    MOV   [gdtr + 2], EAX
#    MOV   AX, [ESP + 8]
#    MOV   [gdtr], AX
#    LGDT  [gdtr]
#    RET

# reloadSegments:
# 	# Reload CS register containing code selector:
# 	JMP   0x08:reload_CS # 0x08 points at the new code selector
# .reload_CS:
# 	# Reload data segment registers:
# 	MOV   AX, 0x10 # 0x10 points at the new data selector
# 	MOV   DS, AX
# 	MOV   ES, AX
# 	MOV   FS, AX
# 	MOV   GS, AX
# 	MOV   SS, AX
# 	RET

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b

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
    push $0
    push $17
    jmp isr_common_stub

#  18: Machine Check Exception (Pentium/586+)
isr18:
    cli
    push $0
    push $18
    jmp isr_common_stub

#  19 to 31: Reserved Exceptions
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

isr30:
    cli
    push $0
    push $30
    jmp isr_common_stub

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

.size _start, . - _start
