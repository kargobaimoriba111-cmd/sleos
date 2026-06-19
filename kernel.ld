/*
 * =============================================================================
 * SLeOS Kernel Linker Script (kernel.ld)
 *
 * Places the kernel entry (from kernel_entry.asm) first, followed by all
 * C code and data.  The kernel is linked to load at physical address 0x1000
 * (KERNEL_OFFSET in boot.asm).
 * =============================================================================
 */

ENTRY(_start)

SECTIONS {
    /*
     * The bootloader copies the kernel to 0x1000 and jumps to it.
     * All sections must be laid out from that address.
     */
    . = 0x1000;

    /* ── Kernel entry stub (must be first) ── */
    .text : {
        *(.text._start)     /* Ensure _start is at the very beginning */
        *(.text)
        *(.text.*)
    }

    /* ── Read-only data ── */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    /* ── Initialised data ── */
    .data : {
        *(.data)
        *(.data.*)
    }

    /* ── Uninitialised data (zeroed by convention — no BSS clear in this kernel) ── */
    .bss : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
}
