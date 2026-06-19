/*
 * =============================================================================
 * SLeOS - PS/2 Keyboard Driver (keyboard.h)
 * Polls the keyboard controller for scan codes and converts them to ASCII.
 * =============================================================================
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../kernel/types.h"

#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_OUTPUT_FULL 0x01   /* bit 0 of status = output buffer full */

/* Initialise keyboard driver */
void keyboard_init(void);

/* Block until a printable key is pressed; return its ASCII character */
char keyboard_getchar(void);

/* Non-blocking: return 0 if no key ready, else ASCII char */
char keyboard_poll(void);

#endif /* KEYBOARD_H */
