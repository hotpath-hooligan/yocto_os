#include "kernel.h"

#include "console.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "serial.h"
#include "vga.h"

#define SERIAL_COM1 0x3F8
#define SERIAL_DIVISOR_38400 3

void kernel_main(void)
{
    gdt_init();
    terminal_initialize();
    serial_initialize(SERIAL_COM1, SERIAL_DIVISOR_38400);

    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Yocto OS\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    console_initialize();

    keyboard_initialize();
    interrupts_initialize();

    serial_log("INFO", "GDT, IDT, PIC, keyboard, and terminal initialized");
    serial_log("INFO", "Keyboard IRQ enabled; entering idle loop");

    cpu_enable_interrupts();
    for (;;) {
        cpu_halt();
    }
}
