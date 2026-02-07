# Terminal OS (C + ASM) Handoff Notes

Date: 2026-02-05

This repo currently boots a floppy image in QEMU (see `sim.bat`), loads a flat kernel at physical `0x10000`, and jumps to it from a stage-2 bootloader.

You asked for:
- Keep the current structure (don’t change `sim.bat`).
- Implement a *simple* scheduler + kernel “shell” (terminal-only).
- Use C + ASM.
- Write down goals/roadmap for a terminal-based Linux-like OS (no GUI).

## What “works now” (current boot path)

- `boot/boot.asm`: BIOS boot sector, loads stage-2 to `0x7E00`.
- `boot/second_stage.asm`: stage-2, loads `outs/kernel.bin` starting at floppy sector 3 into `0x1000:0` (physical `0x10000`), then jumps to `0x1000:0`.
- `kernel/trampoline.asm`: kernel entry (linker `ENTRY(trampoline_start)`), switches to 32-bit protected mode using a minimal GDT.
- `kernel/kernel_entry.asm`: 32-bit entry, calls `kernel_main`.

Important: stage-2 must be able to load kernels larger than a single track. `boot/second_stage.asm` now advances CHS correctly across sectors/heads/cylinders for a standard 1.44MB floppy (18 sectors/track, 2 heads).

## Changes made in this turn

### 1) Make kernel reach C code after entering protected mode

- `kernel/trampoline.asm` now calls `kernel_entry` after setting up protected mode.

### 2) Simple text console + keyboard input (polled)

- `kernel/console.c` + `include/console.h`: basic VGA text output (clear, write, scroll).
- `kernel/kbd.c` + `include/kbd.h`: PS/2 controller polling (`0x64` status + `0x60` data), basic Set-1 scancode → ASCII mapping (letters/numbers/space/enter/backspace).

This avoids IRQ/IDT complexity for now (still terminal-only, minimal moving parts).

### 3) Simple cooperative scheduler + context switching

- `kernel/arch/i386/switch.asm`: `switch_context(old_esp*, new_esp)` implemented as `pushad; save esp; load esp; popad; ret`.
- `kernel/sched.c` + `include/sched.h`: fixed-size task table + per-task stacks + round-robin runnable selection.

This is *cooperative* scheduling:
- Tasks yield explicitly via `sched_yield()`.
- No timer interrupt preemption yet (that’s the next step after solid IRQ/PIT support).

### 4) Minimal shell + a background task

- `kernel/shell.c` + `include/shell.h`: interactive prompt with:
  - `help`, `ps`, `yield`, `clear`
- `kernel/kernel.c`: spawns:
  - `shell` (interactive)
  - `bg-counter` (prints periodic ticks and yields)
  - then starts the scheduler

### 5) Build integration

- `build_kernel.bat` compiles/links the added `.c` files and assembles `kernel/arch/i386/switch.asm`.

## Repo structure (what to keep stable)

- `boot/` = real-mode boot stages
- `kernel/` = kernel C + ASM
- `kernel/arch/i386/` = i386-specific pieces
- `include/` = headers used by kernel C
- `outs/` = generated images/binaries (kernel.bin, floppy.img)

## Immediate next goals (to reach a real terminal OS)

This is the practical progression to get to “Linux-like terminal OS” while keeping the codebase stable.

### Phase 0: Hardening the boot + memory assumptions

- Loader:
  - Support kernels >64KiB (ES:BX wrap handling, or a flat unreal-mode copy, or switch to protected mode in stage-2 and copy via 32-bit).
  - Consider switching to LBA reads (`INT 13h extensions`) to avoid CHS limits.
- Kernel placement:
  - Decide a fixed virtual/physical load scheme (right now the linker is `.` = `0x00` and code uses “relocation-by-offset” patterns in some files).
- Basic sanity:
  - A single “printk” path used everywhere (`console_*`) rather than ad-hoc VGA writes.

### Phase 1: Interrupts + PIT + preemption

- IDT:
  - Build a real IDT in kernel virtual/physical terms (no “guess load offset” tricks).
  - Add ISR/IRQ stubs in ASM that save registers and call C handlers.
- PIC:
  - Remap PIC to `0x20/0x28`, unmask IRQ0 (timer) and IRQ1 (keyboard).
  - Send EOI to PIC in IRQ handlers.
- PIT:
  - Program PIT channel 0 to a known tick rate (e.g. 100Hz).
- Scheduler upgrade:
  - Move from cooperative `sched_yield()` to preemptive scheduling on IRQ0.
  - Define what “context” means across an interrupt (EIP/CS/EFLAGS are on the interrupt stack frame).

### Phase 2: Processes + syscalls (still terminal-only)

- Define kernel/user boundary:
  - Ring 3 user processes, ring 0 kernel.
  - Per-process page tables (once paging exists).
- Syscall ABI:
  - `int 0x80`-style or `sysenter` (later).
  - Minimal syscalls: `read`, `write`, `exit`, `fork` (optional), `exec` (later).
- Process model:
  - PCB (PID, state, kernel stack, user stack, address space).
  - Waiting/blocking (sleep, IO wait).
  - Reaping zombies.

### Phase 3: Multithreading and synchronization

- Kernel threads:
  - Thread struct separate from process struct (shared address space).
- Locks:
  - Spinlock + interrupt disable rules.
  - Later: mutex + condition variables.

### Phase 4: Terminal + job control (shell like Linux)

- Line discipline:
  - Canonical mode, backspace/editing, history (optional).
- Background jobs:
  - Shell creates processes, doesn’t “own” the scheduler.
  - `&` operator, job table, `fg`, `bg`, `jobs`.
- Signals (later):
  - `SIGINT` (Ctrl-C), `SIGTSTP` (Ctrl-Z).

### Phase 5: Filesystem and executables (so “run programs” is real)

- Start with an in-memory FS or FAT12 reader (since you already boot from floppy).
- ELF loader:
  - Load text/data segments, set up user stack, jump to entry.
- Userspace:
  - Minimal libc and a couple tiny utilities.

## Notes for other agents

- Don’t change `sim.bat`. It’s the orchestrator for build + image + QEMU.
- Be careful with kernel size:
  - Loader CHS is now correct, but ES:BX wrap is still a ceiling around 64KiB for stage-2 as written.
- Keep architecture split:
  - Anything i386-specific should stay under `kernel/arch/i386/`.

