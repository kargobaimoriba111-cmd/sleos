; =============================================================================
; SLeOS Kernel Entry - kernel_entry.asm
; Bridges the bootloader (Assembly) and the C kernel
; =============================================================================

[BITS 32]

; Declare external C function
[EXTERN kernel_main]

; Export this symbol so linker can find it
[GLOBAL _start]

_start:
    call kernel_main            ; Call the C kernel main function
    jmp $                       ; Infinite loop if kernel returns (should not happen)
