/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)

_kernel_offset = 0xC0000000;
_kernel_virt_start = 0xC0100000;

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{

  /* The multiboot data and code will exist in low memory
    starting at 0x100000 */

  . = 0x00100000;
  _kernel_phys_start = .;

  .multiboot.data : {
     *(.multiboot.data)
  }

  .multiboot.text : {
     *(.multiboot.text)
  }

  /* The kernel will live at 3GB + 1MB in the virtual address space, */
  /* which will be mapped to 1MB in the physical address space. */
  /* Note that we page-align the sections. */
  . += _kernel_offset;

  .text ALIGN (4K) : AT (ADDR (.text) - _kernel_offset)
  {
  *(.text)
  }
  .rodata ALIGN (4K) : AT (ADDR (.rodata) - _kernel_offset)
  {
  *(.rodata)
  }
  .data ALIGN (4K) : AT (ADDR (.data) - _kernel_offset)
  {
  *(.data)
  }
  .bss ALIGN (4K) : AT (ADDR (.bss) - _kernel_offset)
  {
  *(COMMON)
  *(.bss)
  }

  /DISCARD/ : {
      *(.comment*);
  }

	_kernel_phys_end = . - _kernel_offset;
  _kernel_virt_end = .;

	/* The compiler may produce other sections, put them in the proper place in
	   in this file, if you'd like to include them in the final kernel. */
}
