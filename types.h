/*
 * =============================================================================
 * SLeOS - Fundamental Type Definitions (types.h)
 * Provides fixed-width integer types without relying on stdint.h
 * (which may not be available in a freestanding environment).
 * =============================================================================
 */

#ifndef TYPES_H
#define TYPES_H

/* Unsigned integers */
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;

/* Signed integers */
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;

/* Pointer-sized integer */
typedef unsigned int        uintptr_t;
typedef unsigned int        size_t;

/* Boolean */
typedef int                 bool;
#define true  1
#define false 0

/* NULL */
#ifndef NULL
#define NULL ((void *)0)
#endif

#endif /* TYPES_H */
