/*
 * =============================================================================
 * SLeOS - Memory Manager (memory.h)
 *
 * Implements a simple fixed-size block allocator (pool allocator) to
 * demonstrate memory management concepts.
 *
 * The heap is divided into TOTAL_BLOCKS blocks of BLOCK_SIZE bytes each.
 * A bitmap tracks which blocks are allocated.  The "mem" shell command
 * displays a visual map of free/used memory.
 * =============================================================================
 */

#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* ── Configuration ───────────────────────────────────────────────────────── */
#define HEAP_BASE       0x200000    /* 2 MB — well above kernel code           */
#define BLOCK_SIZE      4096        /* 4 KB per block (matches x86 page size)  */
#define TOTAL_BLOCKS    64          /* 64 blocks = 256 KB simulated heap       */

/* ── Public API ──────────────────────────────────────────────────────────── */
void     memory_init(void);

/* Allocate 'n' contiguous blocks; return base address or 0 on failure */
uint32_t memory_alloc(uint32_t n_blocks);

/* Free 'n' blocks starting at 'addr' */
void     memory_free(uint32_t addr, uint32_t n_blocks);

/* Print visual memory map to screen */
void     memory_print_map(void);

/* Print summary statistics */
void     memory_print_stats(void);

/* Run an automatic demo: alloc, free, show map */
void     memory_demo(void);

/* Return number of free blocks */
uint32_t memory_free_count(void);
uint32_t memory_used_count(void);

#endif /* MEMORY_H */
