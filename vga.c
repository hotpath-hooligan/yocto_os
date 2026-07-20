#include "vga.h"

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static volatile uint16_t* terminal_buffer = (volatile uint16_t*)VGA_MEMORY;

uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background)
{
    return (uint8_t)(foreground | (background << 4));
}

uint16_t vga_entry(unsigned char character, uint8_t color)
{
    return (uint16_t)character | ((uint16_t)color << 8);
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (uint8_t)((inb(0x3D5) & 0xC0) | cursor_start));
    outb(0x3D4, 0x0B);
    outb(0x3D5, (uint8_t)((inb(0x3D5) & 0xE0) | cursor_end));
}

void disable_cursor(void)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void update_cursor(size_t column, size_t row)
{
    uint16_t position = (uint16_t)(row * VGA_WIDTH + column);

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

static void terminal_fill_row(size_t row)
{
    for (size_t column = 0; column < VGA_WIDTH; ++column) {
        terminal_buffer[row * VGA_WIDTH + column] = vga_entry(' ', terminal_color);
    }
}

static void terminal_scroll(void)
{
    for (size_t row = 1; row < VGA_HEIGHT; ++row) {
        for (size_t column = 0; column < VGA_WIDTH; ++column) {
            terminal_buffer[(row - 1) * VGA_WIDTH + column] =
                terminal_buffer[row * VGA_WIDTH + column];
        }
    }

    terminal_fill_row(VGA_HEIGHT - 1);
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_clear(void)
{
    for (size_t row = 0; row < VGA_HEIGHT; ++row) {
        terminal_fill_row(row);
    }

    terminal_row = 0;
    terminal_column = 0;
    update_cursor(terminal_column, terminal_row);
}

void terminal_initialize(void)
{
    terminal_buffer = (volatile uint16_t*)VGA_MEMORY;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_clear();
    enable_cursor(14, 15);
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putentryat(char character, uint8_t color, size_t column, size_t row)
{
    if (column >= VGA_WIDTH || row >= VGA_HEIGHT) {
        return;
    }

    terminal_buffer[row * VGA_WIDTH + column] = vga_entry((unsigned char)character, color);
}

void terminal_putchar(char character)
{
    if (character == '\n') {
        terminal_column = 0;
        ++terminal_row;
    } else if (character == '\r') {
        terminal_column = 0;
    } else if (character == '\t') {
        size_t next_tab_stop = (terminal_column + 4) & ~(size_t)3;
        do {
            terminal_putchar(' ');
        } while (terminal_column != 0 && terminal_column < next_tab_stop);
        return;
    } else {
        terminal_putentryat(character, terminal_color, terminal_column, terminal_row);
        ++terminal_column;
        if (terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            ++terminal_row;
        }
    }

    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }
    update_cursor(terminal_column, terminal_row);
}

void terminal_backspace(void)
{
    if (terminal_column > 0) {
        --terminal_column;
    } else if (terminal_row > 0) {
        --terminal_row;
        terminal_column = VGA_WIDTH - 1;
    } else {
        return;
    }

    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    update_cursor(terminal_column, terminal_row);
}

void terminal_write(const char* data, size_t size)
{
    for (size_t index = 0; index < size; ++index) {
        terminal_putchar(data[index]);
    }
}

void terminal_writestring(const char* data)
{
    while (*data != '\0') {
        terminal_putchar(*data++);
    }
}
