/*
 * =============================================================================
 * SLeOS - VGA Text Mode Screen Driver (screen.h)
 * Provides low-level functions for writing to the 80x25 VGA text buffer.
 * Memory-mapped I/O at 0xB8000.
 * =============================================================================
 */

#ifndef SCREEN_H
#define SCREEN_H

#include "../kernel/types.h"

/* VGA Text Mode Constants */
#define VGA_BASE        0xB8000     /* Physical address of VGA text buffer     */
#define VGA_COLS        80          /* Number of columns                        */
#define VGA_ROWS        25          /* Number of rows                           */
#define VGA_SIZE        (VGA_COLS * VGA_ROWS)

/* VGA Colour Attributes (foreground | background << 4) */
#define BLACK           0x0
#define BLUE            0x1
#define GREEN           0x2
#define CYAN            0x3
#define RED             0x4
#define MAGENTA         0x5
#define BROWN           0x6
#define LIGHT_GREY      0x7
#define DARK_GREY       0x8
#define LIGHT_BLUE      0x9
#define LIGHT_GREEN     0xA
#define LIGHT_CYAN      0xB
#define LIGHT_RED       0xC
#define LIGHT_MAGENTA   0xD
#define YELLOW          0xE
#define WHITE           0xF

/* Macro: combine fg and bg into a VGA attribute byte */
#define VGA_COLOR(fg, bg)   ((bg << 4) | fg)

/* Common colour presets */
#define COLOR_DEFAULT   VGA_COLOR(WHITE,      BLACK)
#define COLOR_HEADER    VGA_COLOR(LIGHT_GREEN, BLACK)
#define COLOR_PROMPT    VGA_COLOR(LIGHT_CYAN,  BLACK)
#define COLOR_ERROR     VGA_COLOR(LIGHT_RED,   BLACK)
#define COLOR_SUCCESS   VGA_COLOR(LIGHT_GREEN, BLACK)
#define COLOR_WARNING   VGA_COLOR(YELLOW,      BLACK)
#define COLOR_INFO      VGA_COLOR(LIGHT_CYAN,  BLACK)
#define COLOR_TITLE     VGA_COLOR(YELLOW,      BLUE)
#define COLOR_BORDER    VGA_COLOR(LIGHT_CYAN,  BLACK)

/* ── Public API ─────────────────────────────────────────────────────────── */
void screen_init(void);
void screen_clear(void);
void screen_set_color(uint8_t color);
uint8_t screen_get_color(void);

void put_char(char c);
void print(const char *str);
void print_color(const char *str, uint8_t color);
void println(const char *str);
void println_color(const char *str, uint8_t color);
void print_int(int n);
void print_hex(uint32_t n);
void print_newline(void);

void cursor_set(int row, int col);
void cursor_get(int *row, int *col);

void print_centered(const char *str, uint8_t color);
void print_line(char c, uint8_t color);    /* prints a full-width line of char c */

#endif /* SCREEN_H */
