# Copilot instructions for OSP (x86_64 toy OS)

This file equips AI coding agents with the minimal, actionable knowledge to be productive in this repository.

## Big picture
- Target: 64-bit x86 (x86_64) toy operating system. Bootflow lives in `boot/` and `kernel/`.
- Bootloader: `boot/boot64.asm` (assembled with `nasm`). Kernel entry and early init in `kernel/kernel_entry.asm` and `kernel/` C sources.
- Userspace: not fully implemented; primary code is a single freestanding kernel built from `kernel/*.c`.
- Linker scripts: `linker64.ld` (used for 64-bit builds referenced from the `Makefile`) and older `linker.ld` for other artifacts.

## Where to look (key files)
- Build orchestration: [Makefile](Makefile#L1) — canonical build/test/run targets (`make all`, `make test`, `make run`).
- Environment setup: [scripts/setup_linux_env.sh](scripts/setup_linux_env.sh#L1) — packages required on Debian/Ubuntu.
- Intended architecture & roadmap: [INTENDEDWORKFLOW.md](INTENDEDWORKFLOW.md#L1).
- GRUB config used to create the ISO: [grub/grub.cfg](grub/grub.cfg#L1).
- Kernel sources: `kernel/` and public headers in `include/`.
- Assembly and arch-specific code: `kernel/arch/i386/` and `boot/`.
- Cross-compiler/build helpers: `cross_compiler/` scripts (if native toolchain is missing).

## Build / test / run (explicit commands)
- Prepare environment (Debian/Ubuntu):
  - `./scripts/setup_linux_env.sh`
- Full build: `make clean && make all` — produces `build/osp.iso`.
- Run interactively: `make run` (launches QEMU with `-serial stdio` and hidden display).
- Automated test: `make test` — runs QEMU for ~8s and asserts log strings. The test checks for exactly the strings `OSP x86_64` and `MMU: 4-level paging enabled` in `build/qemu.log`.
- ISO creation uses `grub-mkrescue` (GRUB + xorriso). Output ISO: `build/osp.iso`.

## Project-specific build conventions
- Compiler flags (see `Makefile`): freestanding build with `-std=gnu11 -ffreestanding -fno-stack-protector -fno-pic -m64 -mno-red-zone -O2` (no libc or dynamic runtime expected).
- Assembly objects are generated with `nasm -f elf64` for `boot64.asm`.
- Linking uses `ld` with `-nostdlib` and the project linker script; do not call `gcc` to link implicitly unless preserving `LDFLAGS` semantics.
- Kernel entrypoints and memory layout are governed by the linker script (`linker64.ld`) — change linker scripts carefully and verify `build/kernel.elf` location.

## Runtime and debugging hints
- QEMU logger: test target writes `build/qemu.log`. Use that for non-interactive CI checks.
- Interactive serial console via `-serial stdio` (so kernel logs and the shell appear in terminal). The `Makefile` sets `-display none -no-reboot`.
- To reproduce manual QEMU runs you can use `qemu-system-x86_64 -cdrom build/osp.iso -serial stdio -display none -no-reboot`.

## Coding patterns and conventions to follow
- Place kernel C files under `kernel/` and headers under `include/`.
- Low-level/arch-specific code goes in `kernel/arch/i386/` (see `gdt.c`, `interrupts.c`, `switch.asm`).
- Minimal runtime: prefer explicit implementations of `memcpy`/`memset` and avoid libc calls — the repository already uses minimal helpers under `kernel/` and `include/`.
- Use `panic()` (see `panic.c`/`panic.h`) for unrecoverable errors; logging is available via `log.c`/`serial.c`.

## Integration points / external deps
- QEMU (qemu-system-x86_64) — used for running and testing.
- grub-mkrescue / xorriso / grub-pc-bin — used to create ISO images.
- nasm, gcc, binutils (ld) — assemble, compile, and link.
- Cross-compiler scripts exist under `cross_compiler/` if you need a dedicated toolchain.

## Examples to reference when changing build or boot code
- Changing boot loader: update `boot/boot64.asm` and rebuild `make all` to regenerate `build/boot64.o`.
- Changing kernel layout: update `linker64.ld` and verify `ld` invocation in `Makefile` produces `build/kernel.elf`.
- Adding a new kernel C file: add `kernel/xyz.c` — `Makefile` has pattern rule `build/%.o: kernel/%.c`.

## Safety notes for AI edits
- Do not assume standard C runtime exists; keep code freestanding and use the provided `include/` headers.
- Avoid changing linker scripts or `Makefile` defaults without running `make all` and `make test` locally in CI/emulator.

---
If any section is unclear or you want specific examples (e.g., a small contribution PR template, or common test failures and fixes), tell me which area to expand.
