#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

void serial_initialize(uint16_t port, uint16_t divisor);
void serial_write_char(char character);
void serial_write(const char* text);
void serial_log(const char* level, const char* message);

#endif /* SERIAL_H */
