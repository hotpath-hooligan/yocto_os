#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

/* This kernel is for i386 targets. */
#if !defined(__i386__)
#error "This kernel needs to be compiled with a i386 compiler"
#endif

/* I/O port functions */
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

static inline void cpu_enable_interrupts(void)
{
    asm volatile ("sti" : : : "memory");
}

static inline void cpu_disable_interrupts(void)
{
    asm volatile ("cli" : : : "memory");
}

static inline void cpu_halt(void)
{
    asm volatile ("hlt" : : : "memory");
}

#endif /* KERNEL_H */
