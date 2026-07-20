#include "idt.h"

#include "keyboard.h"
#include "serial.h"
#include "vga.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20
#define PIC_ICW1_INITIALIZE 0x11
#define PIC_ICW4_8086 0x01
#define PIC1_VECTOR_OFFSET 0x20
#define PIC2_VECTOR_OFFSET 0x28
#define IRQ_COUNT 16
#define KEYBOARD_IRQ 1

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_pointer idt_descriptor;

extern void (*const interrupt_stub_table[48])(void);

_Static_assert(sizeof(struct interrupt_frame) == 17 * sizeof(uint32_t),
               "interrupt frame must match interrupts.s");

static const char* const exception_names[32] = {
    "Divide error",
    "Debug",
    "Non-maskable interrupt",
    "Breakpoint",
    "Overflow",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "x87 floating-point exception",
    "Alignment check",
    "Machine check",
    "SIMD floating-point exception",
    "Virtualization exception",
    "Control-protection exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor injection exception",
    "VMM communication exception",
    "Security exception",
    "Reserved",
};

static void idt_set_entry(uint8_t vector, void (*handler)(void))
{
    uint32_t address = (uint32_t)handler;

    idt[vector].offset_low = (uint16_t)(address & 0xFFFF);
    idt[vector].selector = KERNEL_CODE_SEGMENT;
    idt[vector].zero = 0;
    idt[vector].type_attributes = IDT_INTERRUPT_GATE;
    idt[vector].offset_high = (uint16_t)(address >> 16);
}

static void idt_load(const struct idt_pointer* descriptor)
{
    asm volatile ("lidt (%0)" : : "r"(descriptor) : "memory");
}

static void pic_remap(void)
{
    uint8_t master_mask = inb(PIC1_DATA);
    uint8_t slave_mask = inb(PIC2_DATA);

    outb(PIC1_COMMAND, PIC_ICW1_INITIALIZE);
    io_wait();
    outb(PIC2_COMMAND, PIC_ICW1_INITIALIZE);
    io_wait();
    outb(PIC1_DATA, PIC1_VECTOR_OFFSET);
    io_wait();
    outb(PIC2_DATA, PIC2_VECTOR_OFFSET);
    io_wait();
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();
    outb(PIC1_DATA, PIC_ICW4_8086);
    io_wait();
    outb(PIC2_DATA, PIC_ICW4_8086);
    io_wait();

    outb(PIC1_DATA, master_mask);
    outb(PIC2_DATA, slave_mask);
}

static void pic_mask_all_except_keyboard(void)
{
    outb(PIC1_DATA, (uint8_t)~(1U << KEYBOARD_IRQ));
    outb(PIC2_DATA, 0xFF);
}

static void pic_acknowledge(uint8_t irq)
{
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

static void write_hex32(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";

    for (int shift = 28; shift >= 0; shift -= 4) {
        char digit = digits[(value >> shift) & 0x0F];
        terminal_putchar(digit);
        serial_write_char(digit);
    }
}

static void halt_on_exception(const struct interrupt_frame* frame)
{
    cpu_disable_interrupts();
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("\nKernel exception: ");
    terminal_writestring(exception_names[frame->vector]);
    terminal_writestring("\nVector 0x");
    serial_write("[FATAL] Kernel exception: ");
    serial_write(exception_names[frame->vector]);
    serial_write("; vector=0x");
    write_hex32(frame->vector);
    terminal_writestring("  error 0x");
    serial_write(" error=0x");
    write_hex32(frame->error_code);
    terminal_writestring("  eip 0x");
    serial_write(" eip=0x");
    write_hex32(frame->eip);
    terminal_putchar('\n');
    serial_write("\n");

    for (;;) {
        cpu_halt();
    }
}

void interrupts_initialize(void)
{
    for (size_t index = 0; index < IDT_ENTRIES; ++index) {
        idt[index].offset_low = 0;
        idt[index].selector = 0;
        idt[index].zero = 0;
        idt[index].type_attributes = 0;
        idt[index].offset_high = 0;
    }
    for (uint8_t vector = 0; vector < 48; ++vector) {
        idt_set_entry(vector, interrupt_stub_table[vector]);
    }

    idt_descriptor.limit = (uint16_t)(sizeof(idt) - 1);
    idt_descriptor.base = (uint32_t)idt;

    pic_remap();
    pic_mask_all_except_keyboard();
    idt_load(&idt_descriptor);
}

void interrupt_dispatch(struct interrupt_frame* frame)
{
    if (frame->vector < 32) {
        halt_on_exception(frame);
    }

    if (frame->vector >= PIC1_VECTOR_OFFSET &&
        frame->vector < PIC1_VECTOR_OFFSET + IRQ_COUNT) {
        uint8_t irq = (uint8_t)(frame->vector - PIC1_VECTOR_OFFSET);

        if (irq == KEYBOARD_IRQ) {
            keyboard_handle_scancode(inb(0x60));
        }
        pic_acknowledge(irq);
    }
}
