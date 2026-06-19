/*
 * =============================================================================
 * SLeOS - Memory Manager (memory.c)
 *
 * Fixed-size block pool allocator with visual bitmap output.
 * =============================================================================
 */

#include "memory.h"
#include "../drivers/screen.h"

/* ── Private State ───────────────────────────────────────────────────────── */
static uint8_t  block_bitmap[TOTAL_BLOCKS];   /* 0 = free, 1 = used          */
static uint32_t alloc_sizes[TOTAL_BLOCKS];    /* #blocks allocated at addr   */

/* ── Init ────────────────────────────────────────────────────────────────── */
void memory_init(void)
{
    int i;
    for (i = 0; i < TOTAL_BLOCKS; i++) {
        block_bitmap[i] = 0;
        alloc_sizes[i]  = 0;
    }

    /* Mark first 4 blocks as "kernel reserved" to be realistic */
    block_bitmap[0] = 1;
    block_bitmap[1] = 1;
    block_bitmap[2] = 1;
    block_bitmap[3] = 1;
    alloc_sizes[0]  = 4;
}

/* ── Allocate ────────────────────────────────────────────────────────────── */
uint32_t memory_alloc(uint32_t n_blocks)
{
    if (n_blocks == 0 || n_blocks > TOTAL_BLOCKS) return 0;

    int i, j;
    /* First-Fit search for n_blocks contiguous free blocks */
    for (i = 0; i <= (int)(TOTAL_BLOCKS - n_blocks); i++) {
        int fit = 1;
        for (j = i; j < (int)(i + n_blocks); j++) {
            if (block_bitmap[j]) { fit = 0; break; }
        }
        if (fit) {
            for (j = i; j < (int)(i + n_blocks); j++)
                block_bitmap[j] = 1;
            alloc_sizes[i] = n_blocks;
            return HEAP_BASE + (uint32_t)i * BLOCK_SIZE;
        }
    }
    return 0; /* allocation failed */
}

/* ── Free ────────────────────────────────────────────────────────────────── */
void memory_free(uint32_t addr, uint32_t n_blocks)
{
    if (addr < HEAP_BASE) return;
    uint32_t block_idx = (addr - HEAP_BASE) / BLOCK_SIZE;
    if (block_idx >= TOTAL_BLOCKS) return;

    uint32_t count = n_blocks;
    if (count == 0) count = alloc_sizes[block_idx]; /* use stored size */

    uint32_t i;
    for (i = block_idx; i < block_idx + count && i < TOTAL_BLOCKS; i++)
        block_bitmap[i] = 0;

    alloc_sizes[block_idx] = 0;
}

/* ── Statistics ──────────────────────────────────────────────────────────── */
uint32_t memory_free_count(void)
{
    uint32_t count = 0, i;
    for (i = 0; i < TOTAL_BLOCKS; i++)
        if (!block_bitmap[i]) count++;
    return count;
}

uint32_t memory_used_count(void)
{
    return TOTAL_BLOCKS - memory_free_count();
}

/* ── Visual Memory Map ───────────────────────────────────────────────────── */
void memory_print_map(void)
{
    println_color("", COLOR_DEFAULT);
    println_color("  MEMORY MAP (mem)", COLOR_HEADER);
    print_line('=', COLOR_BORDER);

    print_color("  Heap Base:   ", COLOR_INFO);
    print_hex(HEAP_BASE);
    print("\n");
    print_color("  Block Size:  4096 bytes (4 KB)\n", COLOR_INFO);
    print_color("  Total Blocks: ", COLOR_INFO);
    print_int(TOTAL_BLOCKS);
    print("  (");
    print_int(TOTAL_BLOCKS * 4);
    println(" KB total)");

    print_line('-', COLOR_BORDER);
    println_color("  [#]=USED  [.]=FREE  [K]=KERNEL", COLOR_DEFAULT);
    print_line('-', COLOR_BORDER);

    print("  ");
    int i;
    for (i = 0; i < TOTAL_BLOCKS; i++) {
        if (i > 0 && i % 16 == 0) {
            /* Print address at start of each new row */
            print_newline();
            print("  ");
        }
        if (i < 4) {
            print_color("[K]", COLOR_ERROR);          /* kernel reserved */
        } else if (block_bitmap[i]) {
            print_color("[#]", COLOR_WARNING);        /* allocated       */
        } else {
            print_color("[.]", COLOR_SUCCESS);        /* free            */
        }
    }
    print_newline();
    print_line('-', COLOR_BORDER);
    memory_print_stats();
}

void memory_print_stats(void)
{
    uint32_t free  = memory_free_count();
    uint32_t used  = memory_used_count();
    uint32_t total = TOTAL_BLOCKS;

    print_color("  Used: ", COLOR_WARNING);
    print_int((int)used);
    print(" blocks (");
    print_int((int)(used * 4));
    print(" KB)   Free: ");
    print_color("", COLOR_SUCCESS);
    print_int((int)free);
    print(" blocks (");
    print_int((int)(free * 4));
    println(" KB)");

    /* ASCII bar chart */
    print_color("  Usage: [", COLOR_DEFAULT);
    int bar_used = (int)((used * 40) / total);
    int j;
    for (j = 0; j < 40; j++) {
        if (j < bar_used)
            print_color("#", COLOR_WARNING);
        else
            print_color(".", COLOR_SUCCESS);
    }
    print("] ");
    print_int((int)((used * 100) / total));
    println("%");
}

/* ── Demo ────────────────────────────────────────────────────────────────── */
void memory_demo(void)
{
    println_color("", COLOR_DEFAULT);
    println_color("  MEMORY ALLOCATION DEMO", COLOR_HEADER);
    print_line('=', COLOR_BORDER);

    println_color("  Step 1: Initial state (only kernel blocks reserved)", COLOR_INFO);
    memory_print_stats();

    println_color("\n  Step 2: Allocating 6 blocks for 'EduApp'...", COLOR_INFO);
    uint32_t addr1 = memory_alloc(6);
    print_color("    -> Allocated at: ", COLOR_SUCCESS);
    print_hex(addr1);
    print_newline();
    memory_print_stats();

    println_color("\n  Step 3: Allocating 10 blocks for 'DataCache'...", COLOR_INFO);
    uint32_t addr2 = memory_alloc(10);
    print_color("    -> Allocated at: ", COLOR_SUCCESS);
    print_hex(addr2);
    print_newline();
    memory_print_stats();

    println_color("\n  Step 4: Allocating 4 blocks for 'NetBuf'...", COLOR_INFO);
    uint32_t addr3 = memory_alloc(4);
    print_color("    -> Allocated at: ", COLOR_SUCCESS);
    print_hex(addr3);
    print_newline();

    println_color("\n  Step 5: Freeing 'EduApp' blocks...", COLOR_INFO);
    memory_free(addr1, 6);
    println_color("    -> Freed.", COLOR_SUCCESS);

    println_color("\n  Final memory map after demo:", COLOR_INFO);
    memory_print_map();

    /* Clean up demo allocations */
    memory_free(addr2, 10);
    memory_free(addr3, 4);
}
