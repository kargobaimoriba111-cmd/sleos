# Sierra Leone OS (SLeOS) v1.0

> *"Computing for Every Sierra Leonean"*

A minimal bootable operating system prototype built from scratch in C and x86 Assembly, designed and implemented by **BICT2201 Group 7** at **Limkokwing University, Freetown, Sierra Leone – 2026**.

---

## 📌 Project Overview

SLeOS is an educational OS kernel that boots in QEMU and demonstrates three core operating system components:

| Feature | Algorithm / Method |
|---|---|
| **Process Scheduler** | First-Come First-Served (FCFS) + Round-Robin (RR) |
| **Memory Manager** | Fixed-block pool allocator with visual bitmap |
| **File System** | In-memory flat file system (SLeOS-FS) |
| **Interactive Shell** | Full keyboard-driven REPL |

---

## 🖥️ Boot Screen

```
==============================================================================
               SIERRA LEONE OS (SLeOS) v1.0
     Built by Limkokwing University Students  BICT2201  Group: 7
                Freetown, Sierra Leone  -  2026
==============================================================================

  Kernel Features:
    * Process Scheduler  (FCFS + Round-Robin)
    * Memory Manager     (Fixed-block Pool Allocator)
    * In-Memory File System (SLeOS-FS)
    * Interactive Shell

  Type 'help' for a list of commands.
```

---

## 🚀 How to Build & Run

### Prerequisites

```bash
sudo apt-get install nasm gcc binutils qemu-system-x86
```

### Build

```bash
git clone https://github.com/<your-username>/SLeOS.git
cd SLeOS
make
```

### Run in QEMU

```bash
# SDL/GTK window (recommended)
qemu-system-x86_64 -drive file=sleos.img,format=raw -m 128M

# Curses terminal mode
make run
```

---

## 🛠️ Shell Commands

### System
| Command | Description |
|---|---|
| `help` | Show all commands |
| `clear` | Clear screen |
| `uname` | OS version information |
| `about` | SLeOS mission & real-world context |
| `halt` | Shutdown SLeOS |

### Process Scheduling
| Command | Description |
|---|---|
| `ps` | Display process table |
| `sched fcfs` | Run FCFS scheduling simulation |
| `sched rr` | Run Round-Robin simulation (quantum = 2) |
| `sched reset` | Reset all processes |

### Memory Management
| Command | Description |
|---|---|
| `mem` | Display visual memory block map |
| `memtest` | Run allocation/free demo |

### File System
| Command | Description |
|---|---|
| `ls` | List all files |
| `cat <file>` | Read file contents |
| `touch <file>` | Create new file |
| `write <file> <data>` | Append data to file |
| `rm <file>` | Delete file |
| `stat <file>` | File metadata |
| `fsinfo` | Filesystem summary |

---

## 📁 Project Structure

```
SLeOS/
├── boot/
│   └── boot.asm           # MBR bootloader (512 bytes, NASM)
├── kernel/
│   ├── kernel_entry.asm   # ASM→C bridge (_start → kernel_main)
│   ├── kernel.c            # Kernel main + subsystem init
│   ├── scheduler.c/h       # FCFS + Round-Robin process scheduler
│   ├── memory.c/h          # Fixed-block pool memory manager
│   ├── types.h             # Freestanding type definitions
│   └── port_io.h           # x86 IN/OUT port wrappers
├── drivers/
│   ├── screen.c/h          # VGA text mode driver (0xB8000)
│   └── keyboard.c/h        # PS/2 keyboard polling driver
├── fs/
│   └── filesystem.c/h      # In-memory flat file system
├── shell/
│   └── shell.c/h           # Interactive command shell (REPL)
├── tools/
│   └── kernel.ld           # GNU LD linker script
├── Makefile                 # Build system
└── README.md
```

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────┐
│                  BIOS / Firmware                │
└──────────────────────┬──────────────────────────┘
                       │ Boot (int 0x19)
┌──────────────────────▼──────────────────────────┐
│         Bootloader (boot.asm) — Sector 0        │
│  • Loads kernel sectors 1-32 into 0x1000        │
│  • Sets up GDT, switches to 32-bit Protected    │
│    Mode, jumps to kernel_entry                  │
└──────────────────────┬──────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────┐
│      Kernel Entry (kernel_entry.asm)            │
│      └─► kernel_main() [C]                     │
│           ├── screen_init()   (VGA driver)      │
│           ├── keyboard_init() (PS/2)            │
│           ├── memory_init()   (pool allocator)  │
│           ├── scheduler_init()(FCFS+RR)         │
│           ├── fs_init()       (SLeOS-FS)        │
│           └── shell_run()     (REPL loop) ◄──┐  │
│                    │                         │  │
│               read command                  │  │
│               parse + dispatch              │  │
│               call subsystem               ──┘  │
└─────────────────────────────────────────────────┘
```

---

## 🌍 Real-World Relevance to Sierra Leone

| Local Challenge | SLeOS Response |
|---|---|
| Limited internet access | Fully offline — no network required |
| High software licensing costs | 100% free and open source |
| Aging/low-spec hardware | Runs on any x86 PC (< 2 MB RAM needed) |
| Educational computing gap | Exposes OS internals to BICT students |
| Power instability | Small footprint, boots in seconds |
| Rural school digital access | USB-bootable, no installation needed |

---

## 🔧 Technical Specifications

- **Architecture:** x86 32-bit Protected Mode
- **Bootloader:** Custom MBR (512 bytes, NASM)
- **Language:** C (freestanding, no libc) + x86 Assembly
- **Toolchain:** GCC i386, NASM, GNU LD
- **Emulator:** QEMU x86_64
- **Memory Model:** Flat 32-bit, kernel at 0x1000
- **VGA Mode:** Text mode 80×25, 0xB8000

---

## 👥 Team — BICT2201 Group 7

Limkokwing University of Creative Technology  
Freetown, Sierra Leone — 2026

---

## 📄 License

MIT License — free to use, study, and modify.
