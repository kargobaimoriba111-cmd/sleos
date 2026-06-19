/*
 * =============================================================================
 * SLeOS - VGA Text Mode Screen Driver (screen.c)
 * =============================================================================
 */

#include "screen.h"
#include "../kernel/types.h"
#include "../kernel/port_io.h"

/* ── Private state ───────────────────────────────────────────────────────── */
static volatile uint16_t *vga_buf = (uint16_t *)VGA_BASE;
static int cursor_row = 0;
static int cursor_col = 0;
static uint8_t current_color = COLOR_DEFAULT;

/* ── VGA hardware cursor ─────────────────────────────────────────────────── */
static void hw_cursor_update(void)
{
    uint16_t pos = cursor_row * VGA_COLS + cursor_col;
    port_byte_out(0x3D4, 0x0F);
    port_byte_out(0x3D5, (uint8_t)(pos & 0xFF));
    port_byte_out(0x3D4, 0x0E);
    port_byte_out(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/* ── Scroll one line upward ──────────────────────────────────────────────── */
static void scroll(void)
{
    uint16_t blank = (uint16_t)(((uint16_t)current_color << 8) | ' ');
    int i;

    /* Move every row up by one */
    for (i = 0; i < (VGA_ROWS - 1) * VGA_COLS; i++)
        vga_buf[i] = vga_buf[i + VGA_COLS];

    /* Clear the last row */
    for (i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++)
        vga_buf[i] = blank;

    cursor_row = VGA_ROWS - 1;
}

/* ── Public: Initialise screen ───────────────────────────────────────────── */
void screen_init(void)
{
    current_color = COLOR_DEFAULT;
    screen_clear();
}

void screen_clear(void)
{
    uint16_t blank = (uint16_t)(((uint16_t)current_color << 8) | ' ');
    int i;
    for (i = 0; i < VGA_SIZE; i++)
        vga_buf[i] = blank;
    cursor_row = 0;
    cursor_col = 0;
    hw_cursor_update();
}

void screen_set_color(uint8_t color) { current_color = color; }
uint8_t screen_get_color(void)       { return current_color; }

/* ── Core character output ───────────────────────────────────────────────── */
void put_char(char c)
{
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\r') {
        cursor_col = 0;
    } else if (c == '\t') {
        cursor_col = (cursor_col + 8) & ~7;  /* align to 8-column tab stop */
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            int offset = cursor_row * VGA_COLS + cursor_col;
            vga_buf[offset] = (uint16_t)(((uint16_t)current_color << 8) | ' ');
        }
    } else {
        int offset = cursor_row * VGA_COLS + cursor_col;
        vga_buf[offset] = (uint16_t)(((uint16_t)current_color << 8) | (uint8_t)c);
        cursor_col++;
    }

    /* Wrap to next line if past right edge */
    if (cursor_col >= VGA_COLS) {
        cursor_col = 0;
        cursor_row++;
    }

    /* Scroll if past bottom */
    if (cursor_row >= VGA_ROWS)
        scroll();

    hw_cursor_update();
}

void print(const char *str)
{
    while (*str) put_char(*str++);
}

void print_color(const char *str, uint8_t color)
{
    uint8_t saved = current_color;
    current_color = color;
    print(str);
    current_color = saved;
}

void println(const char *str)
{
    print(str);
    put_char('\n');
}

void println_color(const char *str, uint8_t color)
{
    print_color(str, color);
    put_char('\n');
}

void print_newline(void) { put_char('\n'); }

void print_int(int n)
{
    char buf[12];
    int i = 0;
    int neg = 0;

    if (n < 0) { neg = 1; n = -n; }
    if (n == 0) { put_char('0'); return; }

    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    if (neg) buf[i++] = '-';

    while (i > 0) put_char(buf[--i]);
}

void print_hex(uint32_t n)
{
    const char *hex = "0123456789ABCDEF";
    print("0x");
    int i;
    for (i = 28; i >= 0; i -= 4)
        put_char(hex[(n >> i) & 0xF]);
}

void cursor_set(int row, int col)
{
    cursor_row = row;
    cursor_col = col;
    hw_cursor_update();
}

void cursor_get(int *row, int *col)
{
    if (row) *row = cursor_row;
    if (col) *col = cursor_col;
}

/* ── Utilities ───────────────────────────────────────────────────────────── */
void print_centered(const char *str, uint8_t color)
{
    /* Measure length */
    int len = 0;
    const char *p = str;
    while (*p++) len++;

    int pad = (VGA_COLS - len) / 2;
    if (pad < 0) pad = 0;

    uint8_t saved = current_color;
    current_color = color;

    int i;
    for (i = 0; i < pad; i++) put_char(' ');
    print(str);
    put_char('\n');

    current_color = saved;
}

void print_line(char c, uint8_t color)
{
    uint8_t saved = current_color;
    current_color = color;
    int i;
    for (i = 0; i < VGA_COLS; i++) put_char(c);
    current_color = saved;
}
