# =============================================================================
# SLeOS Makefile
# Sierra Leone OS v1.0 — BICT2201 Group 7
# Builds a bootable .img using NASM + GCC (i386 freestanding) + LD
# =============================================================================

# ── Toolchain ─────────────────────────────────────────────────────────────
ASM     := nasm
CC      := gcc
LD      := ld

# ── Flags ─────────────────────────────────────────────────────────────────
# -m32            : generate 32-bit code
# -ffreestanding  : no host libc
# -fno-pie        : disable position-independent executables
# -nostdlib       : no standard library startup files
# -O2             : optimise
CFLAGS  := -m32 -ffreestanding -fno-pie -fno-stack-protector \
           -nostdlib -nostdinc -fno-builtin \
           -O2 -Wall -Wextra \
           -I. -Ikernel -Idrivers -Ifs -Ishell

LDFLAGS := -m elf_i386 -T tools/kernel.ld --oformat binary

# ── Directories ───────────────────────────────────────────────────────────
BUILD   := build
IMG     := sleos.img

# ── Source Files ──────────────────────────────────────────────────────────
BOOT_SRC  := boot/boot.asm
ENTRY_SRC := kernel/kernel_entry.asm

C_SRCS    := kernel/kernel.c    \
             kernel/scheduler.c \
             kernel/memory.c    \
             drivers/screen.c   \
             drivers/keyboard.c \
             fs/filesystem.c    \
             shell/shell.c

# ── Object Files ──────────────────────────────────────────────────────────
BOOT_OBJ  := $(BUILD)/boot.bin
ENTRY_OBJ := $(BUILD)/kernel_entry.o
C_OBJS    := $(C_SRCS:%.c=$(BUILD)/%.o)
KERNEL_BIN:= $(BUILD)/kernel.bin

# ── Default target ────────────────────────────────────────────────────────
.PHONY: all clean run dirs

all: dirs $(IMG)
	@echo ""
	@echo "=========================================="
	@echo "  SLeOS build complete!"
	@echo "  Image: $(IMG)"
	@echo "  Run with:"
	@echo "  qemu-system-x86_64 -drive file=$(IMG),format=raw"
	@echo "=========================================="

# ── Create build directories ──────────────────────────────────────────────
dirs:
	@mkdir -p $(BUILD)/kernel $(BUILD)/drivers $(BUILD)/fs $(BUILD)/shell

# ── Assemble bootloader ───────────────────────────────────────────────────
$(BOOT_OBJ): $(BOOT_SRC)
	@echo "[ASM] $<"
	$(ASM) -f bin -o $@ $<

# ── Assemble kernel entry ─────────────────────────────────────────────────
$(ENTRY_OBJ): $(ENTRY_SRC)
	@echo "[ASM] $<"
	$(ASM) -f elf32 -o $@ $<

# ── Compile C source files ────────────────────────────────────────────────
$(BUILD)/%.o: %.c
	@echo "[CC ] $<"
	$(CC) $(CFLAGS) -c -o $@ $<

# ── Link kernel ───────────────────────────────────────────────────────────
$(KERNEL_BIN): $(ENTRY_OBJ) $(C_OBJS)
	@echo "[LD ] Linking kernel..."
	$(LD) $(LDFLAGS) -o $@ $^

# ── Combine bootloader + kernel into disk image ───────────────────────────
$(IMG): $(BOOT_OBJ) $(KERNEL_BIN)
	@echo "[IMG] Creating disk image..."
	# Create a blank 1.44 MB floppy image
	dd if=/dev/zero of=$(IMG) bs=512 count=2880 2>/dev/null
	# Write bootloader to sector 0
	dd if=$(BOOT_OBJ) of=$(IMG) conv=notrunc bs=512 count=1 2>/dev/null
	# Write kernel starting at sector 1 (offset 512 bytes)
	dd if=$(KERNEL_BIN) of=$(IMG) conv=notrunc bs=512 seek=1 2>/dev/null
	@echo "[IMG] $(IMG) ready ($(shell du -h $(IMG) | cut -f1))"

# ── Run in QEMU ───────────────────────────────────────────────────────────
run: all
	qemu-system-x86_64 \
		-drive file=$(IMG),format=raw,if=floppy \
		-m 128M \
		-display curses \
		-no-reboot

run-gtk: all
	qemu-system-x86_64 \
		-drive file=$(IMG),format=raw,if=floppy \
		-m 128M \
		-display gtk \
		-no-reboot

run-sdl: all
	qemu-system-x86_64 \
		-drive file=$(IMG),format=raw \
		-m 128M \
		-display sdl \
		-no-reboot

# ── Clean ─────────────────────────────────────────────────────────────────
clean:
	@echo "[CLN] Cleaning..."
	rm -rf $(BUILD) $(IMG)

# ── Print info ────────────────────────────────────────────────────────────
info:
	@echo "SLeOS Build Info"
	@echo "  ASM:   $(ASM)"
	@echo "  CC:    $(CC)"
	@echo "  LD:    $(LD)"
	@echo "  Image: $(IMG)"
