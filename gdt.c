#include "gdt.h"

/* GDT entries */
static struct gdt_entry gdt[3];
static struct gdt_ptr gdt_pointer;

void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
{
    gdt[index].base_low = (base & 0xFFFF);
    gdt[index].base_middle = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;
    
    gdt[index].limit_low = (limit & 0xFFFF);
    gdt[index].granularity = (limit >> 16) & 0x0F;
    gdt[index].granularity |= granularity & 0xF0;
    
    gdt[index].access = access;
}

void gdt_init(void)
{
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gdt_pointer.base = (uint32_t)&gdt;
    
    /* NULL descriptor (index 0) */
    gdt_set_entry(0, 0, 0, 0, 0);
    
    /* Kernel code segment (index 1) */
    gdt_set_entry(1, 0, 0xFFFFFFFF, 
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_SYSTEM | 
                  GDT_ACCESS_EXECUTABLE | GDT_ACCESS_READABLE, 
                  GDT_GRAN_4K | GDT_GRAN_32BIT | GDT_GRAN_LIMIT_HIGH);
    
    /* Kernel data segment (index 2) */
    gdt_set_entry(2, 0, 0xFFFFFFFF,
                  GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_SYSTEM | 
                  GDT_ACCESS_WRITABLE,
                  GDT_GRAN_4K | GDT_GRAN_32BIT | GDT_GRAN_LIMIT_HIGH);
    
    gdt_flush(&gdt_pointer);
}

/* Assembly function to load GDT and update segment registers */
void gdt_flush(const struct gdt_ptr* gdt_ptr)
{
    asm volatile (
        "lgdt (%0)        \n\t"  // Load GDT
        "mov $0x10, %%ax  \n\t"  // Load kernel data segment (0x10)
        "mov %%ax, %%ds   \n\t"  // Set data segment
        "mov %%ax, %%es   \n\t"  // Set extra segment
        "mov %%ax, %%fs   \n\t"  // Set FS segment
        "mov %%ax, %%gs   \n\t"  // Set GS segment
        "mov %%ax, %%ss   \n\t"  // Set stack segment
        "ljmp $0x08, $1f  \n\t"  // Far jump to reload code segment
        "1:               \n\t"
        :
        : "r" (gdt_ptr)
        : "eax", "memory"
    );
}
