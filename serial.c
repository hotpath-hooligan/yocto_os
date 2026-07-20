#include "serial.h"

#include "kernel.h"

#define SERIAL_COM1 0x3F8
#define SERIAL_DATA 0
#define SERIAL_INTERRUPT_ENABLE 1
#define SERIAL_FIFO_CONTROL 2
#define SERIAL_LINE_CONTROL 3
#define SERIAL_MODEM_CONTROL 4
#define SERIAL_LINE_STATUS 5
#define SERIAL_DLAB 0x80
#define SERIAL_TRANSMITTER_EMPTY 0x20
#define SERIAL_WAIT_LIMIT 100000U

static uint16_t serial_port = SERIAL_COM1;

void serial_initialize(uint16_t port, uint16_t divisor)
{
    serial_port = port;

    outb((uint16_t)(port + SERIAL_INTERRUPT_ENABLE), 0x00);
    outb((uint16_t)(port + SERIAL_LINE_CONTROL), SERIAL_DLAB);
    outb((uint16_t)(port + SERIAL_DATA), (uint8_t)(divisor & 0xFF));
    outb((uint16_t)(port + SERIAL_INTERRUPT_ENABLE), (uint8_t)(divisor >> 8));
    outb((uint16_t)(port + SERIAL_LINE_CONTROL), 0x03);
    outb((uint16_t)(port + SERIAL_FIFO_CONTROL), 0xC7);
    outb((uint16_t)(port + SERIAL_MODEM_CONTROL), 0x0B);
}

static int serial_transmitter_ready(void)
{
    return (inb((uint16_t)(serial_port + SERIAL_LINE_STATUS)) &
            SERIAL_TRANSMITTER_EMPTY) != 0;
}

void serial_write_char(char character)
{
    unsigned int attempts = SERIAL_WAIT_LIMIT;

    while (!serial_transmitter_ready() && attempts > 0) {
        --attempts;
    }
    if (attempts > 0) {
        outb(serial_port, (uint8_t)character);
    }
}

void serial_write(const char* text)
{
    while (*text != '\0') {
        if (*text == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(*text++);
    }
}

void serial_log(const char* level, const char* message)
{
    serial_write("[");
    serial_write(level);
    serial_write("] ");
    serial_write(message);
    serial_write("\n");
}
