/*
 * =============================================================================
 * SLeOS - Kernel Main (kernel.c)
 *
 * Entry point called from kernel_entry.asm after the bootloader sets up
 * protected mode.  Initialises all subsystems and starts the shell.
 * =============================================================================
 */

#include "types.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "scheduler.h"
#include "memory.h"
#include "../fs/filesystem.h"
#include "../shell/shell.h"

/*
 * kernel_main() — called by kernel_entry.asm
 * This function must never return.
 */
void kernel_main(void)
{
    /* ── 1. Initialise screen first so we can display any early errors ── */
    screen_init();
    screen_set_color(COLOR_DEFAULT);

    /* ── 2. Display early boot message ── */
    println_color("  SLeOS Kernel Loading...", COLOR_HEADER);

    /* ── 3. Initialise keyboard driver ── */
    keyboard_init();
    println_color("  [OK] Keyboard driver initialised", COLOR_SUCCESS);

    /* ── 4. Initialise memory manager ── */
    memory_init();
    println_color("  [OK] Memory manager initialised", COLOR_SUCCESS);

    /* ── 5. Initialise process scheduler ── */
    scheduler_init();
    println_color("  [OK] Process scheduler initialised", COLOR_SUCCESS);

    /* ── 6. Initialise file system ── */
    fs_init();
    println_color("  [OK] Filesystem initialised (SLeOS-FS)", COLOR_SUCCESS);

    /* ── 7. Initialise shell ── */
    shell_init();
    println_color("  [OK] Shell initialised", COLOR_SUCCESS);

    println_color("  All subsystems nominal. Starting shell...", COLOR_INFO);

    /* Brief display before clearing to shell banner */
    volatile int i;
    for (i = 0; i < 5000000; i++);

    /* ── 8. Hand control to the interactive shell — does not return ── */
    shell_run();

    /*
     * Should never reach here.
     * If shell_run() somehow returns, halt the CPU.
     */
    __asm__ __volatile__("cli; hlt");
    while (1) {}
}
