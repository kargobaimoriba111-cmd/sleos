/*
 * =============================================================================
 * SLeOS - Simple In-Memory File System (filesystem.c)
 * =============================================================================
 */

#include "filesystem.h"
#include "../drivers/screen.h"

/* ── Private State ───────────────────────────────────────────────────────── */
static FSFile   fs_table[FS_MAX_FILES];
static uint32_t fs_tick_counter = 0;

/* Simple string utilities (no libc) */
static int kstrcmp(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static void kstrcpy(char *dst, const char *src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
}

static int kstrlen(const char *s)
{
    int i = 0;
    while (s[i]) i++;
    return i;
}

static void kmemcpy(uint8_t *dst, const uint8_t *src, int n)
{
    int i;
    for (i = 0; i < n; i++) dst[i] = src[i];
}

/* Find file index by name; return -1 if not found */
static int find_file(const char *name)
{
    int i;
    for (i = 0; i < FS_MAX_FILES; i++)
        if (fs_table[i].in_use && kstrcmp(fs_table[i].name, name) == 0)
            return i;
    return -1;
}

/* ── Init ────────────────────────────────────────────────────────────────── */
void fs_init(void)
{
    int i;
    for (i = 0; i < FS_MAX_FILES; i++) {
        fs_table[i].in_use        = false;
        fs_table[i].size          = 0;
        fs_table[i].created_time  = 0;
        fs_table[i].modified_time = 0;
    }
    fs_tick_counter = 0;

    /* Seed with some demonstration files */
    fs_create("readme.txt");
    fs_write("readme.txt",
        "Sierra Leone OS (SLeOS) v1.0\n"
        "Built by BICT2201 Group 7 - Limkokwing University\n"
        "Freetown, Sierra Leone - 2026\n");

    fs_create("mission.txt");
    fs_write("mission.txt",
        "SLeOS Mission:\n"
        "To provide low-resource computing for Sierra Leone's\n"
        "education system and digital inclusion initiatives.\n"
        "Designed for offline use in schools without internet.\n");

    fs_create("schools.csv");
    fs_write("schools.csv",
        "School,District,Students,Computers\n"
        "Bo Secondary,Bo,1200,8\n"
        "Kenema Poly,Kenema,900,5\n"
        "Port Loko High,Port Loko,780,3\n");
}

/* ── Create ──────────────────────────────────────────────────────────────── */
int fs_create(const char *name)
{
    if (find_file(name) != -1) return -1;   /* already exists */

    int i;
    for (i = 0; i < FS_MAX_FILES; i++) {
        if (!fs_table[i].in_use) {
            kstrcpy(fs_table[i].name, name, FS_MAX_FILENAME);
            fs_table[i].size          = 0;
            fs_table[i].in_use        = true;
            fs_table[i].created_time  = fs_tick_counter;
            fs_table[i].modified_time = fs_tick_counter;
            return 0;
        }
    }
    return -1; /* no free slots */
}

/* ── Read ────────────────────────────────────────────────────────────────── */
int fs_read(const char *name)
{
    int idx = find_file(name);
    if (idx == -1) return -1;

    FSFile *f = &fs_table[idx];
    println_color("", COLOR_DEFAULT);
    print_color("  --- ", COLOR_BORDER);
    print_color(name, COLOR_TITLE);
    print_color(" ---\n", COLOR_BORDER);

    if (f->size == 0) {
        println_color("  (empty file)", COLOR_WARNING);
    } else {
        /* Print file contents character by character */
        uint32_t j;
        print("  ");
        for (j = 0; j < f->size; j++) {
            char c = (char)f->data[j];
            if (c == '\n') {
                put_char('\n');
                print("  ");
            } else {
                put_char(c);
            }
        }
        print_newline();
    }

    print_color("  [", COLOR_BORDER);
    print_int((int)f->size);
    println(" bytes]");
    return 0;
}

/* ── Write (append) ──────────────────────────────────────────────────────── */
int fs_write(const char *name, const char *data)
{
    int idx = find_file(name);
    if (idx == -1) return -1;

    FSFile *f  = &fs_table[idx];
    int    len = kstrlen(data);

    if ((int)(f->size) + len > FS_MAX_FILESIZE)
        len = FS_MAX_FILESIZE - (int)f->size;   /* truncate */

    kmemcpy(f->data + f->size, (const uint8_t *)data, len);
    f->size          += (uint32_t)len;
    f->modified_time  = fs_tick_counter;
    return 0;
}

/* ── Delete ──────────────────────────────────────────────────────────────── */
int fs_delete(const char *name)
{
    int idx = find_file(name);
    if (idx == -1) return -1;
    fs_table[idx].in_use = false;
    fs_table[idx].size   = 0;
    return 0;
}

/* ── List (ls) ───────────────────────────────────────────────────────────── */
void fs_list(void)
{
    println_color("", COLOR_DEFAULT);
    println_color("  FILESYSTEM LISTING (ls)", COLOR_HEADER);
    print_line('=', COLOR_BORDER);
    print_color("  NAME                          SIZE    CREATED\n", COLOR_INFO);
    print_line('-', COLOR_BORDER);

    int count = 0, i;
    for (i = 0; i < FS_MAX_FILES; i++) {
        FSFile *f = &fs_table[i];
        if (!f->in_use) continue;

        print("  ");
        print_color(f->name, COLOR_TITLE);

        /* Pad name to 30 chars */
        int len = kstrlen(f->name);
        int j;
        for (j = len; j < 30; j++) put_char(' ');

        print_int((int)f->size);
        print(" B");

        /* Pad size to 8 chars */
        int slen = 0, sz = (int)f->size;
        while (sz > 0) { sz /= 10; slen++; }
        if (f->size == 0) slen = 1;
        for (j = slen + 2; j < 8; j++) put_char(' ');

        print_color("t=", COLOR_DEFAULT);
        print_int((int)f->created_time);
        print_newline();
        count++;
    }

    if (count == 0)
        println_color("  (no files)", COLOR_WARNING);

    print_line('-', COLOR_BORDER);
    print_color("  Total: ", COLOR_INFO);
    print_int(count);
    print(" file(s) | Capacity: ");
    print_int(FS_MAX_FILES);
    println(" slots");
}

/* ── Stat ────────────────────────────────────────────────────────────────── */
int fs_stat(const char *name)
{
    int idx = find_file(name);
    if (idx == -1) return -1;

    FSFile *f = &fs_table[idx];
    println_color("", COLOR_DEFAULT);
    print_color("  STAT: ", COLOR_HEADER);
    println(name);
    print_line('-', COLOR_BORDER);
    print_color("  Name:     ", COLOR_INFO); println(f->name);
    print_color("  Size:     ", COLOR_INFO); print_int((int)f->size); println(" bytes");
    print_color("  Created:  ", COLOR_INFO); print("t="); print_int((int)f->created_time); print_newline();
    print_color("  Modified: ", COLOR_INFO); print("t="); print_int((int)f->modified_time); print_newline();
    print_color("  Status:   ", COLOR_INFO);
    println_color(f->in_use ? "Active" : "Deleted", COLOR_SUCCESS);
    return 0;
}

/* ── FS Info ─────────────────────────────────────────────────────────────── */
void fs_info(void)
{
    println_color("", COLOR_DEFAULT);
    println_color("  FILESYSTEM INFO", COLOR_HEADER);
    print_line('-', COLOR_BORDER);
    print_color("  Label:      ", COLOR_INFO); println(FS_DISK_LABEL);
    print_color("  Type:       ", COLOR_INFO); println("SLeOS In-Memory FS v1");
    print_color("  Max files:  ", COLOR_INFO); print_int(FS_MAX_FILES); print_newline();
    print_color("  File slots: ", COLOR_INFO);
    print_int(fs_file_count()); print(" used / ");
    print_int(FS_MAX_FILES - fs_file_count()); println(" free");
    print_color("  Block size: ", COLOR_INFO); println("512 bytes");
    print_color("  Location:   ", COLOR_INFO); println("RAM (volatile — data lost on reboot)");
}

int fs_file_count(void)
{
    int count = 0, i;
    for (i = 0; i < FS_MAX_FILES; i++)
        if (fs_table[i].in_use) count++;
    return count;
}

void fs_tick(void) { fs_tick_counter++; }
