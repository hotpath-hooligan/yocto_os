#ifndef GDT_H
#define GDT_H

#include "kernel.h"

/* GDT Entry Structure */
struct gdt_entry {
    uint16_t limit_low;      // Lower 16 bits of limit
    uint16_t base_low;       // Lower 16 bits of base
    uint8_t  base_middle;    // Next 8 bits of base
    uint8_t  access;         // Access flags
    uint8_t  granularity;    // Granularity and upper limit bits
    uint8_t  base_high;      // Upper 8 bits of base
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;          // Upper 16 bits of all selector limits
    uint32_t base;           // Address of the first gdt_entry struct
} __attribute__((packed));

/* GDT Access byte flags */
#define GDT_ACCESS_PRESENT     0x80    // Present bit
#define GDT_ACCESS_RING0       0x00    // Ring 0 (kernel mode)
#define GDT_ACCESS_RING3       0x60    // Ring 3 (user mode)
#define GDT_ACCESS_SYSTEM      0x10    // System/Code-Data segment
#define GDT_ACCESS_EXECUTABLE  0x08    // Executable (code segment)
#define GDT_ACCESS_READABLE    0x02    // Readable (code segment)
#define GDT_ACCESS_WRITABLE    0x02    // Writable (data segment)

/* GDT Granularity byte flags */
#define GDT_GRAN_4K            0x80    // 4KB granularity
#define GDT_GRAN_32BIT         0x40    // 32-bit protected mode
#define GDT_GRAN_LIMIT_HIGH    0x0F    // Upper 4 bits of limit mask

/* Segment selectors */
#define KERNEL_CODE_SEGMENT    0x08    // GDT entry 1
#define KERNEL_DATA_SEGMENT    0x10    // GDT entry 2

/* Function declarations */
void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);
void gdt_init(void);
void gdt_flush(const struct gdt_ptr* gdt_ptr);

#endif /* GDT_H */
