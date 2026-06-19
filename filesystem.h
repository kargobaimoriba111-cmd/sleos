/*
 * =============================================================================
 * SLeOS - Simple In-Memory File System (filesystem.h)
 *
 * A minimal flat (no directories) file system stored entirely in RAM.
 * Supports: create, read, write (append), delete, list (ls), stat.
 * Designed to illustrate file system concepts without requiring real storage.
 * =============================================================================
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../kernel/types.h"

/* ── Configuration ───────────────────────────────────────────────────────── */
#define FS_MAX_FILES        16
#define FS_MAX_FILENAME     32
#define FS_MAX_FILESIZE     512     /* bytes per file */
#define FS_DISK_LABEL       "SLeOS-FS"

/* ── File Entry ──────────────────────────────────────────────────────────── */
typedef struct {
    char     name[FS_MAX_FILENAME];
    uint8_t  data[FS_MAX_FILESIZE];
    uint32_t size;
    bool     in_use;
    uint32_t created_time;   /* simulated tick counter */
    uint32_t modified_time;
} FSFile;

/* ── Public API ──────────────────────────────────────────────────────────── */
void  fs_init(void);

/* Create a new empty file; return 0 on success, -1 on failure */
int   fs_create(const char *name);

/* Read file contents to screen */
int   fs_read(const char *name);

/* Write (append) 'data' to file */
int   fs_write(const char *name, const char *data);

/* Delete a file */
int   fs_delete(const char *name);

/* List all files (ls) */
void  fs_list(void);

/* Print file metadata (stat) */
int   fs_stat(const char *name);

/* Print overall FS info */
void  fs_info(void);

/* Return file count */
int   fs_file_count(void);

/* Increment tick (call from main loop or shell) */
void  fs_tick(void);

#endif /* FILESYSTEM_H */
