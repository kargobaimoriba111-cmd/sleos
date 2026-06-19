/*
 * =============================================================================
 * SLeOS - x86 Port I/O Helpers (port_io.h / port_io.c)
 * Thin wrappers around the IN/OUT x86 instructions.
 * =============================================================================
 */

#ifndef PORT_IO_H
#define PORT_IO_H

#include "types.h"

/* Write one byte to a hardware I/O port */
static inline void port_byte_out(uint16_t port, uint8_t data)
{
    __asm__ __volatile__("outb %0, %1" : : "a"(data), "Nd"(port));
}

/* Read one byte from a hardware I/O port */
static inline uint8_t port_byte_in(uint16_t port)
{
    uint8_t result;
    __asm__ __volatile__("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* Write one word (2 bytes) to a hardware I/O port */
static inline void port_word_out(uint16_t port, uint16_t data)
{
    __asm__ __volatile__("outw %0, %1" : : "a"(data), "Nd"(port));
}

/* Read one word from a hardware I/O port */
static inline uint16_t port_word_in(uint16_t port)
{
    uint16_t result;
    __asm__ __volatile__("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* Small I/O delay — write to unused port 0x80 */
static inline void io_wait(void)
{
    __asm__ __volatile__("outb %%al, $0x80" : : "a"(0));
}

#endif /* PORT_IO_H */
