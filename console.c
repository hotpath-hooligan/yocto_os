#include "console.h"

#include <stddef.h>
#include <stdint.h>

#include "kernel.h"
#include "serial.h"
#include "vga.h"

#define COMMAND_CAPACITY 64
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_COMMAND_PORT 0x64
#define KEYBOARD_INPUT_BUFFER_FULL 0x02
#define KEYBOARD_RESET_COMMAND 0xFE

static char command[COMMAND_CAPACITY];
static size_t command_length;

static int strings_equal(const char* left, const char* right)
{
    while (*left != '\0' && *right != '\0') {
        if (*left++ != *right++) {
            return 0;
        }
    }
    return *left == *right;
}

static int starts_with(const char* text, const char* prefix)
{
    while (*prefix != '\0') {
        if (*text++ != *prefix++) {
            return 0;
        }
    }
    return 1;
}

static void write_prompt(void)
{
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("yocto> ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

static void reboot(void)
{
    cpu_disable_interrupts();
    while ((inb(KEYBOARD_STATUS_PORT) & KEYBOARD_INPUT_BUFFER_FULL) != 0) {
        /* Wait until the controller can accept the reset command. */
    }
    outb(KEYBOARD_COMMAND_PORT, KEYBOARD_RESET_COMMAND);

    for (;;) {
        cpu_halt();
    }
}

static void execute_command(void)
{
    if (command[0] == '\0') {
        return;
    }
    if (strings_equal(command, "help")) {
        terminal_writestring("Commands: help, about, echo <text>, clear, reboot\n");
    } else if (strings_equal(command, "about")) {
        terminal_writestring("Yocto OS: a small interrupt-driven i386 kernel.\n");
    } else if (strings_equal(command, "clear")) {
        terminal_clear();
    } else if (strings_equal(command, "reboot")) {
        serial_log("INFO", "Reboot requested from console");
        reboot();
    } else if (strings_equal(command, "echo")) {
        terminal_putchar('\n');
    } else if (starts_with(command, "echo ")) {
        terminal_writestring(command + 5);
        terminal_putchar('\n');
    } else {
        terminal_writestring("Unknown command: ");
        terminal_writestring(command);
        terminal_writestring("\nType 'help' for available commands.\n");
    }
}

void console_initialize(void)
{
    command_length = 0;
    command[0] = '\0';
    terminal_writestring("Type 'help' for available commands.\n");
    write_prompt();
}

void console_input_character(char character)
{
    if (command_length >= COMMAND_CAPACITY - 1) {
        return;
    }

    command[command_length++] = character;
    command[command_length] = '\0';
    terminal_putchar(character);
}

void console_backspace(void)
{
    if (command_length == 0) {
        return;
    }

    --command_length;
    command[command_length] = '\0';
    terminal_backspace();
}

void console_submit(void)
{
    terminal_putchar('\n');
    command[command_length] = '\0';
    execute_command();
    command_length = 0;
    command[0] = '\0';
    write_prompt();
}

void console_clear(void)
{
    command_length = 0;
    command[0] = '\0';
    terminal_clear();
    write_prompt();
}
