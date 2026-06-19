; =============================================================================
; SLeOS Bootloader - boot.asm
; Sierra Leone OS v1.0
; BICT2201 Group 7 - Limkokwing University
; =============================================================================
; This is a two-stage aware bootloader. It:
;   1. Sets up segment registers and stack
;   2. Loads the kernel from disk (sector 2 onward)
;   3. Switches to 32-bit Protected Mode
;   4. Jumps to the kernel entry point
; =============================================================================

[BITS 16]
[ORG 0x7C00]

KERNEL_OFFSET equ 0x1000      ; Memory address where kernel is loaded

start:
    ; -------------------------------------------------------------------------
    ; Step 1: Initialise segment registers and stack
    ; -------------------------------------------------------------------------
    cli                         ; Disable interrupts
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; Stack grows downward from bootloader
    sti                         ; Re-enable interrupts

    mov [BOOT_DRIVE], dl        ; BIOS stores boot drive in DL — save it

    ; -------------------------------------------------------------------------
    ; Step 2: Print boot message (Real Mode, BIOS int 10h)
    ; -------------------------------------------------------------------------
    mov si, MSG_BOOT
    call print_string_rm

    ; -------------------------------------------------------------------------
    ; Step 3: Load kernel from disk into memory at KERNEL_OFFSET
    ; -------------------------------------------------------------------------
    call load_kernel

    ; -------------------------------------------------------------------------
    ; Step 4: Switch to Protected Mode
    ; -------------------------------------------------------------------------
    call switch_to_pm

    jmp $                       ; Should never reach here

; =============================================================================
; Real-Mode: Print null-terminated string via BIOS int 10h
; =============================================================================
print_string_rm:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

; =============================================================================
; Real-Mode: Load kernel sectors from disk
;   Loads 32 sectors (16 KB) starting at sector 2 into KERNEL_OFFSET
; =============================================================================
load_kernel:
    mov si, MSG_LOAD
    call print_string_rm

    mov bx, KERNEL_OFFSET       ; ES:BX = destination buffer
    mov dh, 32                  ; Number of sectors to read
    mov dl, [BOOT_DRIVE]        ; Drive number
    call disk_load
    ret

; =============================================================================
; Real-Mode: Read DH sectors from drive DL into ES:BX
; =============================================================================
disk_load:
    push dx
    mov ah, 0x02                ; BIOS Read Sectors
    mov al, dh                  ; Number of sectors
    mov ch, 0x00                ; Cylinder 0
    mov dh, 0x00                ; Head 0
    mov cl, 0x02                ; Start at sector 2 (sector 1 = bootloader)
    int 0x13                    ; BIOS disk interrupt
    jc disk_error               ; Carry flag set = error
    pop dx
    cmp dh, al
    jne disk_error
    ret

disk_error:
    mov si, MSG_DISK_ERR
    call print_string_rm
    jmp $

; =============================================================================
; Protected Mode: GDT Definition
; =============================================================================
gdt_start:
    ; Null descriptor (required)
    dd 0x0
    dd 0x0

    ; Code segment descriptor: base=0, limit=4GB, 32-bit, ring 0, executable
gdt_code:
    dw 0xFFFF           ; Limit low
    dw 0x0000           ; Base low
    db 0x00             ; Base middle
    db 10011010b        ; Access: present, ring0, code, readable
    db 11001111b        ; Flags: 4KB granularity, 32-bit
    db 0x00             ; Base high

    ; Data segment descriptor: base=0, limit=4GB, 32-bit, ring 0, writable
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b        ; Access: present, ring0, data, writable
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size - 1
    dd gdt_start                 ; GDT base address

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; =============================================================================
; Switch to 32-bit Protected Mode
; =============================================================================
switch_to_pm:
    cli                         ; Must disable interrupts
    lgdt [gdt_descriptor]       ; Load GDT

    mov eax, cr0
    or eax, 0x1                 ; Set PE (Protection Enable) bit
    mov cr0, eax

    jmp CODE_SEG:init_pm        ; Far jump flushes CPU pipeline

[BITS 32]
init_pm:
    ; Set up 32-bit data segments
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000            ; Update stack to safe 32-bit location
    mov esp, ebp

    call KERNEL_OFFSET          ; Jump into kernel!

    jmp $                       ; Halt if kernel returns

; =============================================================================
; Data
; =============================================================================
[BITS 16]
MSG_BOOT     db 'SLeOS Bootloader v1.0 - Starting...', 0x0D, 0x0A, 0
MSG_LOAD     db 'Loading kernel from disk...', 0x0D, 0x0A, 0
MSG_DISK_ERR db 'ERROR: Disk read failed!', 0x0D, 0x0A, 0
BOOT_DRIVE   db 0

; =============================================================================
; Boot Sector Padding and Signature
; =============================================================================
times 510-($-$$) db 0           ; Pad to 510 bytes
dw 0xAA55                       ; Boot signature (required by BIOS)
