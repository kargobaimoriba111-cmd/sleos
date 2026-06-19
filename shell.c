/*
 * =============================================================================
 * SLeOS - Interactive Shell (shell.c)
 * =============================================================================
 */

#include "shell.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../kernel/scheduler.h"
#include "../kernel/memory.h"
#include "../fs/filesystem.h"

/* ── Constants ───────────────────────────────────────────────────────────── */
#define CMD_BUF_SIZE    128
#define HISTORY_SIZE    8

/* ── Private State ───────────────────────────────────────────────────────── */
static char  cmd_buf[CMD_BUF_SIZE];
static int   cmd_len = 0;

/* Simple command history */
static char  history[HISTORY_SIZE][CMD_BUF_SIZE];
static int   history_count = 0;
static int   history_idx   = -1;

/* ── String Utilities ────────────────────────────────────────────────────── */
static int kstrlen(const char *s)
{
    int i = 0; while (s[i]) i++; return i;
}

static int kstrcmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static int kstrncmp(const char *a, const char *b, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
        if (!a[i]) return 0;
    }
    return 0;
}

static void kstrcpy(char *dst, const char *src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
}

/* Skip leading spaces */
static const char *skip_spaces(const char *s)
{
    while (*s == ' ') s++;
    return s;
}

/* ── Print Banner ────────────────────────────────────────────────────────── */
static void print_banner(void)
{
    screen_clear();

    print_line('=', COLOR_BORDER);
    print_centered("SIERRA LEONE OS (SLeOS) v1.0", COLOR_TITLE);
    print_centered("Built by Limkokwing University Students  BICT2201  Group: 7", COLOR_HEADER);
    print_centered("Freetown, Sierra Leone  -  2026", COLOR_INFO);
    print_line('=', COLOR_BORDER);

    print_newline();
    println_color("  Kernel Features:", COLOR_HEADER);
    println_color("    * Process Scheduler  (FCFS + Round-Robin)", COLOR_DEFAULT);
    println_color("    * Memory Manager     (Fixed-block Pool Allocator)", COLOR_DEFAULT);
    println_color("    * In-Memory File System (SLeOS-FS)", COLOR_DEFAULT);
    println_color("    * Interactive Shell", COLOR_DEFAULT);
    print_newline();
    println_color("  Type 'help' for a list of commands.", COLOR_WARNING);
    print_newline();
    print_line('-', COLOR_BORDER);
    print_newline();
}

/* ── Print Help ──────────────────────────────────────────────────────────── */
static void cmd_help(void)
{
    println_color("", COLOR_DEFAULT);
    print_line('=', COLOR_BORDER);
    println_color("  SLeOS SHELL COMMANDS", COLOR_HEADER);
    print_line('=', COLOR_BORDER);

    println_color("  SYSTEM:", COLOR_TITLE);
    println_color("    help              - Show this help screen", COLOR_DEFAULT);
    println_color("    clear             - Clear the screen", COLOR_DEFAULT);
    println_color("    uname             - Display OS information", COLOR_DEFAULT);
    println_color("    about             - SLeOS mission and context", COLOR_DEFAULT);
    println_color("    halt              - Shutdown SLeOS", COLOR_DEFAULT);
    print_newline();
    println_color("  PROCESS SCHEDULING:", COLOR_TITLE);
    println_color("    ps                - Show process table", COLOR_DEFAULT);
    println_color("    sched fcfs        - Run FCFS scheduling simulation", COLOR_DEFAULT);
    println_color("    sched rr          - Run Round-Robin simulation", COLOR_DEFAULT);
    println_color("    sched reset       - Reset process table", COLOR_DEFAULT);
    print_newline();
    println_color("  MEMORY MANAGEMENT:", COLOR_TITLE);
    println_color("    mem               - Show memory map & statistics", COLOR_DEFAULT);
    println_color("    memtest           - Run memory allocation demo", COLOR_DEFAULT);
    print_newline();
    println_color("  FILE SYSTEM:", COLOR_TITLE);
    println_color("    ls                - List all files", COLOR_DEFAULT);
    println_color("    cat <file>        - Read file contents", COLOR_DEFAULT);
    println_color("    touch <file>      - Create new empty file", COLOR_DEFAULT);
    println_color("    write <f> <data>  - Append data to file", COLOR_DEFAULT);
    println_color("    rm <file>         - Delete a file", COLOR_DEFAULT);
    println_color("    stat <file>       - Show file metadata", COLOR_DEFAULT);
    println_color("    fsinfo            - Filesystem summary", COLOR_DEFAULT);

    print_line('=', COLOR_BORDER);
}

/* ── Command: uname ──────────────────────────────────────────────────────── */
static void cmd_uname(void)
{
    println_color("", COLOR_DEFAULT);
    print_line('-', COLOR_BORDER);
    println_color("  OS:           Sierra Leone OS (SLeOS)", COLOR_HEADER);
    println_color("  Version:      1.0", COLOR_DEFAULT);
    println_color("  Arch:         x86 (32-bit Protected Mode)", COLOR_DEFAULT);
    println_color("  Kernel:       Monolithic, bare-metal C/ASM", COLOR_DEFAULT);
    println_color("  Emulator:     QEMU x86_64", COLOR_DEFAULT);
    println_color("  Compiler:     GCC (cross-compiled i686-elf)", COLOR_DEFAULT);
    println_color("  Bootloader:   Custom MBR (512 bytes, NASM)", COLOR_DEFAULT);
    println_color("  University:   Limkokwing University, Freetown", COLOR_DEFAULT);
    println_color("  Team:         BICT2201 Group 7, 2026", COLOR_DEFAULT);
    print_line('-', COLOR_BORDER);
}

/* ── Command: about ──────────────────────────────────────────────────────── */
static void cmd_about(void)
{
    println_color("", COLOR_DEFAULT);
    print_line('=', COLOR_BORDER);
    println_color("  ABOUT SLeOS", COLOR_HEADER);
    print_line('-', COLOR_BORDER);
    println_color("  Sierra Leone faces significant digital infrastructure challenges:", COLOR_DEFAULT);
    println_color("    - Limited internet access in rural and peri-urban areas", COLOR_DEFAULT);
    println_color("    - High cost of commercial operating systems (Windows, macOS)", COLOR_DEFAULT);
    println_color("    - Power instability making cloud computing unreliable", COLOR_DEFAULT);
    println_color("    - Schools often have 1-5 aging computers for hundreds of students", COLOR_DEFAULT);
    print_newline();
    println_color("  SLeOS addresses these by being:", COLOR_DEFAULT);
    println_color("    * FREE & open source — zero licensing cost", COLOR_DEFAULT);
    println_color("    * MINIMAL — runs on hardware as old as Pentium 4", COLOR_DEFAULT);
    println_color("    * OFFLINE-FIRST — no internet required after installation", COLOR_DEFAULT);
    println_color("    * EDUCATIONAL — exposes OS internals, teaches BICT students", COLOR_DEFAULT);
    print_newline();
    println_color("  Real-world targets:", COLOR_DEFAULT);
    println_color("    > Primary schools in Bo, Kenema, Port Loko districts", COLOR_DEFAULT);
    println_color("    > University computer labs (Limkokwing, UniMak, Fourah Bay)", COLOR_DEFAULT);
    println_color("    > NGO digital literacy programmes across Freetown", COLOR_DEFAULT);
    print_line('=', COLOR_BORDER);
}

/* ── Command: halt ───────────────────────────────────────────────────────── */
static void cmd_halt(void)
{
    screen_clear();
    print_line('=', COLOR_BORDER);
    print_centered("SLeOS Shutdown", COLOR_HEADER);
    print_line('=', COLOR_BORDER);
    print_newline();
    println_color("  Flushing file system...", COLOR_WARNING);
    println_color("  Releasing memory blocks...", COLOR_WARNING);
    println_color("  Stopping scheduler...", COLOR_WARNING);
    print_newline();
    println_color("  Thank you for using Sierra Leone OS (SLeOS) v1.0", COLOR_SUCCESS);
    println_color("  Built with pride in Freetown, Sierra Leone - 2026", COLOR_INFO);
    println_color("  BICT2201 Group 7 - Limkokwing University", COLOR_INFO);
    print_newline();
    print_line('=', COLOR_BORDER);
    println_color("  System halted. Safe to power off.", COLOR_WARNING);

    /* Halt the CPU */
    __asm__ __volatile__("cli; hlt");
    while (1) {}
}

/* ── Parse and Execute Command ───────────────────────────────────────────── */
static void execute(const char *cmd)
{
    cmd = skip_spaces(cmd);
    int len = kstrlen(cmd);
    if (len == 0) return;

    fs_tick();

    /* ── help ── */
    if (kstrcmp(cmd, "help") == 0) {
        cmd_help();
    }
    /* ── clear ── */
    else if (kstrcmp(cmd, "clear") == 0) {
        print_banner();
    }
    /* ── uname ── */
    else if (kstrcmp(cmd, "uname") == 0) {
        cmd_uname();
    }
    /* ── about ── */
    else if (kstrcmp(cmd, "about") == 0) {
        cmd_about();
    }
    /* ── halt / shutdown ── */
    else if (kstrcmp(cmd, "halt") == 0 || kstrcmp(cmd, "shutdown") == 0) {
        cmd_halt();
    }
    /* ── ps ── */
    else if (kstrcmp(cmd, "ps") == 0) {
        scheduler_print_table();
    }
    /* ── sched ── */
    else if (kstrncmp(cmd, "sched", 5) == 0) {
        const char *arg = skip_spaces(cmd + 5);
        if (kstrcmp(arg, "fcfs") == 0)
            scheduler_run_fcfs();
        else if (kstrcmp(arg, "rr") == 0)
            scheduler_run_rr();
        else if (kstrcmp(arg, "reset") == 0) {
            scheduler_reset();
            println_color("  Scheduler reset.", COLOR_SUCCESS);
        } else {
            println_color("  Usage: sched <fcfs|rr|reset>", COLOR_ERROR);
        }
    }
    /* ── mem ── */
    else if (kstrcmp(cmd, "mem") == 0) {
        memory_print_map();
    }
    /* ── memtest ── */
    else if (kstrcmp(cmd, "memtest") == 0) {
        memory_demo();
    }
    /* ── ls ── */
    else if (kstrcmp(cmd, "ls") == 0) {
        fs_list();
    }
    /* ── cat <file> ── */
    else if (kstrncmp(cmd, "cat ", 4) == 0) {
        const char *fname = skip_spaces(cmd + 4);
        if (fs_read(fname) == -1) {
            print_color("  cat: file not found: ", COLOR_ERROR);
            println(fname);
        }
    }
    /* ── touch <file> ── */
    else if (kstrncmp(cmd, "touch ", 6) == 0) {
        const char *fname = skip_spaces(cmd + 6);
        if (fs_create(fname) == 0) {
            print_color("  Created: ", COLOR_SUCCESS);
            println(fname);
        } else {
            print_color("  touch: file exists or filesystem full: ", COLOR_ERROR);
            println(fname);
        }
    }
    /* ── write <file> <data> ── */
    else if (kstrncmp(cmd, "write ", 6) == 0) {
        const char *rest = skip_spaces(cmd + 6);
        /* Extract filename (up to next space) */
        char fname[32];
        int fi = 0;
        while (*rest && *rest != ' ' && fi < 31)
            fname[fi++] = *rest++;
        fname[fi] = '\0';
        const char *data = skip_spaces(rest);

        if (fi == 0) {
            println_color("  Usage: write <file> <data>", COLOR_ERROR);
        } else if (fs_write(fname, data) == -1) {
            print_color("  write: file not found: ", COLOR_ERROR);
            println(fname);
        } else {
            println_color("  Written.", COLOR_SUCCESS);
        }
    }
    /* ── rm <file> ── */
    else if (kstrncmp(cmd, "rm ", 3) == 0) {
        const char *fname = skip_spaces(cmd + 3);
        if (fs_delete(fname) == 0) {
            print_color("  Deleted: ", COLOR_SUCCESS);
            println(fname);
        } else {
            print_color("  rm: file not found: ", COLOR_ERROR);
            println(fname);
        }
    }
    /* ── stat <file> ── */
    else if (kstrncmp(cmd, "stat ", 5) == 0) {
        const char *fname = skip_spaces(cmd + 5);
        if (fs_stat(fname) == -1) {
            print_color("  stat: file not found: ", COLOR_ERROR);
            println(fname);
        }
    }
    /* ── fsinfo ── */
    else if (kstrcmp(cmd, "fsinfo") == 0) {
        fs_info();
    }
    /* ── Unknown ── */
    else {
        print_color("  Unknown command: '", COLOR_ERROR);
        print(cmd);
        println_color("'  (type 'help' for commands)", COLOR_ERROR);
    }
}

/* ── Read a line from keyboard ───────────────────────────────────────────── */
static void read_line(void)
{
    cmd_len     = 0;
    history_idx = history_count; /* reset history navigation */
    cmd_buf[0]  = '\0';

    while (1) {
        char c = keyboard_getchar();

        if (c == '\n' || c == '\r') {
            /* Enter — submit command */
            cmd_buf[cmd_len] = '\0';
            put_char('\n');

            /* Save to history */
            if (cmd_len > 0) {
                kstrcpy(history[history_count % HISTORY_SIZE],
                        cmd_buf, CMD_BUF_SIZE);
                history_count++;
            }
            return;
        }

        if (c == '\b') {
            /* Backspace */
            if (cmd_len > 0) {
                cmd_len--;
                cmd_buf[cmd_len] = '\0';
                put_char('\b');
            }
            continue;
        }

        /* Printable character */
        if (c >= 32 && c < 127 && cmd_len < CMD_BUF_SIZE - 1) {
            cmd_buf[cmd_len++] = c;
            cmd_buf[cmd_len]   = '\0';
            put_char(c);
        }
    }
}

/* ── Shell Init & Run ────────────────────────────────────────────────────── */
void shell_init(void)
{
    history_count = 0;
    history_idx   = 0;
}

void shell_run(void)
{
    print_banner();

    while (1) {
        /* Print prompt */
        print_color("  sleos", COLOR_PROMPT);
        print_color("@", COLOR_DEFAULT);
        print_color("freetown", COLOR_SUCCESS);
        print_color(":~$ ", COLOR_PROMPT);

        read_line();
        execute(cmd_buf);
        print_newline();
    }
}
