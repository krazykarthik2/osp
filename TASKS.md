# TEJA OS — Product Requirements Checklist

## 1. Boot & Architecture
- [ ] GRUB-based bootloader (Multiboot2 compliant)
- [x] Enter x86_64 long mode
- [ ] Higher-half kernel mapping
- [x] GDT setup
- [ ] IDT setup
- [ ] Interrupt handling framework
- [ ] APIC initialization (Local + IO APIC)
- [ ] ACPI parsing
- [ ] SMP multi-core initialization (bring-up of APs)

---

## 2. Memory Management
- [ ] Physical Memory Manager (bitmap or buddy allocator)
- [x] 4-level paging (PML4)
- [ ] Kernel/User virtual memory separation
- [ ] Per-process address space
- [ ] Demand paging (optional enhancement)
- [ ] Copy-on-write (advanced)
- [ ] Kernel heap allocator (slab or kmalloc)
- [ ] Stack protection with guard pages
- [ ] Stack canary value = 0xDEADC0DE

---

## 3. Caching Subsystem
- [ ] Page cache for filesystem reads
- [ ] Write-back caching mechanism
- [ ] Cache eviction policy (LRU or CLOCK)
- [ ] Buffer cache for block devices
- [ ] Cache statistics interface (cache hits/misses)

---

## 4. Process & Scheduling
- [x] Process control block (PCB)
- [x] Kernel threads
- [ ] User processes (Ring 3)
- [x] Context switching
- [x] Preemptive multitasking
- [x] SMP-aware scheduler
- [x] Per-core run queues
- [ ] Background task support
- [ ] Zombie process handling
- [ ] Crash isolation (process-level fault handling)

---

## 5. Dynamic Program Loading
- [ ] ELF64 loader
- [ ] Support for relocatable binaries
- [ ] Dynamic linking support (.so)
- [ ] Runtime symbol resolution
- [ ] exec() implementation
- [ ] fork() implementation
- [ ] Crash handler (segfault detection)
- [x] Kernel panic mechanism (with stack trace)

---

## 6. Filesystem & Disk
- [x] Block device abstraction
- [ ] SATA/AHCI driver
- [x] VFS (Virtual File System layer)
- [x] Real disk-based filesystem (EXT2/EXT4 Subset)
- [ ] Journaling support
- [ ] Mount/unmount system
- [ ] File permissions
- [x] Directory traversal
- [ ] File descriptor abstraction

---

## 7. Syscall Interface
- [ ] syscall/sysret instruction support
- [ ] System call table
- [ ] read()
- [ ] write()
- [ ] open()
- [ ] close()
- [ ] fork()
- [ ] exec()
- [ ] exit()
- [ ] wait()
- [ ] pipe()

---

## 8. Terminal & Shell
- [x] Text-mode terminal driver (VGA text mode)
- [x] Keyboard driver (PS/2)
- [ ] Terminal emulator abstraction
- [x] Command parser
- [x] Built-in commands (ls, cd, tree, rm, ps, kill, clear)
- [ ] Piping support (|)
- [ ] I/O redirection (>, <)
- [x] Background execution (&)
- [ ] Script execution (.sh style)
- [ ] Environment variables

---

## 9. Userland
- [ ] libc-like minimal standard library
- [ ] Dynamic program execution
- [ ] User/kernel memory protection
- [ ] Per-process stack with guard page
- [ ] Signal handling (SIGSEGV, SIGKILL)

---

## 10. Kernel Reliability
- [ ] Structured kernel panic screen
- [ ] Stack trace printing
- [ ] Per-process crash logging
- [x] Debug logging over serial
- [ ] Watchdog timer (optional)

---

## 11. Security & Protection
- [ ] Ring 0 / Ring 3 separation
- [ ] NX bit enforcement
- [ ] Stack canary validation
- [ ] User pointer validation
- [ ] Kernel memory isolation

---

## 12. Build & Tooling
- [x] Cross compiler (x86_64-elf-gcc)
- [x] QEMU test environment
- [ ] GDB debugging support
- [x] Makefile build system
- [x] Automated build script

---

# Final Product Identity

Name: TEJA OS  
Architecture: x86_64  
Boot Method: GRUB  
Mode: Text-only (no graphics)  
Core Focus: SMP, dynamic loading, caching, robustness
