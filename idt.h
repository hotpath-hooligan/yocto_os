#ifndef IDT_H
#define IDT_H

#include "kernel.h"

#define IDT_ENTRIES 256
#define IDT_INTERRUPT_GATE 0x8E
#define KERNEL_CODE_SEGMENT 0x08

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attributes;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_pointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Exact stack layout produced by interrupts.s before entering C. */
struct interrupt_frame {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t vector;
    uint32_t error_code;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((packed));

void interrupts_initialize(void);
void interrupt_dispatch(struct interrupt_frame* frame);

#endif /* IDT_H */
