// Used for creating GDT segment descriptors in 64-bit integer form.
#include <stdio.h>
#include <stdint.h>

/* Defines a GDT entry. We say packed, because it prevents the
*  compiler from doing things that it thinks is best: Prevent
*  compiler "optimization" by packing */
struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

/* Special pointer which includes the limit: The max bytes
*  taken up by the GDT, minus 1. Again, this NEEDS to be packed */
struct gdt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));


/* This will be a function in start.asm. We use this to properly
*  reload the new segment registers */
extern void gdt_flush();

/* Our GDT, with 3 entries, and finally our special GDT pointer */
struct gdt_entry gdt[3];
struct gdt_ptr gp;

/* Setup a descriptor in the Global Descriptor Table */
void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
    /* Setup the descriptor base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Setup the descriptor limits */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    /* Finally, set up the granularity and access flags */
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

/* Should be called by main. This will setup the special GDT
*  pointer, set up the first 3 entries in our GDT, and then
*  finally call gdt_flush() in our assembler file in order
*  to tell the processor where the new GDT is and update the
*  new segment registers */
void gdt_install()
{
    /* Setup the GDT pointer and limit */
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = &gdt;

    /* Our NULL descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* The second entry is our Code Segment. The base address
    *  is 0, the limit is 4GBytes, it uses 4KByte granularity,
    *  uses 32-bit opcodes, and is a Code Segment descriptor.
    *  Please check the table above in the tutorial in order
    *  to see exactly what each value means */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* The third entry is our Data Segment. It's EXACTLY the
    *  same as our code segment, but the descriptor type in
    *  this entry's access byte says it's a Data Segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Flush out the old GDT and install the new changes! */
    gdt_flush();
}




// /**
// * \param target A pointer to the 8-byte GDT entry
// * \param source An arbitrary structure describing the GDT entry
// */
// void encodeGdtEntry(uint8_t *target, struct GDT source)
// {
//   // Check the limit to make sure that it can be encoded
//   if ((source.limit > 65536) && ((source.limit & 0xFFF) != 0xFFF)) {
//     kerror("You can't do that!");
//   }
//   if (source.limit > 65536) {
//     // Adjust granularity if required
//     source.limit = source.limit >> 12;
//     target[6] = 0xC0;
//   } else {
//     target[6] = 0x40;
//   }
//
//   // Encode the limit
//   target[0] = source.limit & 0xFF;
//   target[1] = (source.limit >> 8) & 0xFF;
//   target[6] |= (source.limit >> 16) & 0xF;
//
//   // Encode the base
//   target[2] = source.base & 0xFF;
//   target[3] = (source.base >> 8) & 0xFF;
//   target[4] = (source.base >> 16) & 0xFF;
//   target[7] = (source.base >> 24) & 0xFF;
//
//   // And... Type
//   target[5] = source.type;
// }
//
// void create_descriptor(uint32_t base, uint32_t limit, uint16_t flag)
// {
//     uint64_t descriptor;
//
//     // Create the high 32 bit segment
//     descriptor  =  limit       & 0x000F0000;         // set limit bits 19:16
//     descriptor |= (flag <<  8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
//     descriptor |= (base >> 16) & 0x000000FF;         // set base bits 23:16
//     descriptor |=  base        & 0xFF000000;         // set base bits 31:24
//
//     // Shift by 32 to allow for low part of segment
//     descriptor <<= 32;
//
//     // Create the low 32 bit segment
//     descriptor |= base  << 16;                       // set base bits 15:0
//     descriptor |= limit  & 0x0000FFFF;               // set limit bits 15:0
//
//     printf("0x%.16llX\n", descriptor);
// }
//
// int main(void)
// {
//     create_descriptor(0, 0, 0);
//     create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL0));
//     create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL0));
//     create_descriptor(0, 0x000FFFFF, (GDT_CODE_PL3));
//     create_descriptor(0, 0x000FFFFF, (GDT_DATA_PL3));
//
//     return 0;
// }
