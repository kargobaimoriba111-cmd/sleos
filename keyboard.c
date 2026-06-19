/*
 * =============================================================================
 * SLeOS - PS/2 Keyboard Driver (keyboard.c)
 * Polling-based driver — no IRQ required.
 * =============================================================================
 */

#include "keyboard.h"
#include "screen.h"
#include "../kernel/port_io.h"

/*
 * US QWERTY scan-code-set-1 → ASCII table.
 * Index = scan code; value = ASCII character (0 = non-printable / modifier).
 */
static const char SCANCODE_TABLE[128] = {
/*00*/  0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
/*0F*/  '\t','q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
/*1D*/  0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'','`',
/*2A*/  0,   '\\','z','x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
/*37*/  '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/*44*/  0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+',
/*53*/  '1', '2', '3', '0', '.', 0,   0,   0,   0,   0,
/*5D*/  [0x39] = ' '   /* Space, already covered above but explicit */
};

static const char SCANCODE_SHIFT[128] = {
/*00*/  0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
/*0F*/  '\t','Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
/*1D*/  0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
/*2A*/  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
/*37*/  '*', 0,   ' '
};

/* Modifier key state */
static int shift_pressed = 0;
static int caps_lock     = 0;

void keyboard_init(void)
{
    shift_pressed = 0;
    caps_lock     = 0;
}

static char scancode_to_ascii(uint8_t sc)
{
    /* Key-release events have bit 7 set */
    if (sc & 0x80) {
        uint8_t released = sc & 0x7F;
        if (released == 0x2A || released == 0x36)
            shift_pressed = 0;
        return 0;
    }

    /* Track modifiers */
    if (sc == 0x2A || sc == 0x36) { shift_pressed = 1; return 0; }
    if (sc == 0x3A) { caps_lock = !caps_lock;          return 0; }

    if (sc >= 128) return 0;

    char c;
    if (shift_pressed)
        c = SCANCODE_SHIFT[sc];
    else
        c = SCANCODE_TABLE[sc];

    /* Apply caps lock to letters only */
    if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
    if (caps_lock && c >= 'A' && c <= 'Z') c += 32;

    return c;
}

char keyboard_poll(void)
{
    if (!(port_byte_in(KEYBOARD_STATUS_PORT) & KEYBOARD_OUTPUT_FULL))
        return 0;
    uint8_t sc = port_byte_in(KEYBOARD_DATA_PORT);
    return scancode_to_ascii(sc);
}

char keyboard_getchar(void)
{
    char c;
    do {
        while (!(port_byte_in(KEYBOARD_STATUS_PORT) & KEYBOARD_OUTPUT_FULL))
            ;   /* spin-wait */
        uint8_t sc = port_byte_in(KEYBOARD_DATA_PORT);
        c = scancode_to_ascii(sc);
    } while (c == 0);
    return c;
}
