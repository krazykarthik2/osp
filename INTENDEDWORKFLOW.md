# MODULES

This document lists the intended modules for a sample operating system. Use this as a roadmap for development and prioritization.

---

1. Boot System 🔌

- Bootloader
- CPU mode initialization
- Stack setup
- Kernel loading
- Jump to kernel entry
- Architecture Abstraction
  - x86 boot path (Bochs)
  - ARM64 boot path (Pi)
- Linker & Memory Layout
  - Kernel sections
  - Stack regions
  - Heap regions

---

2. Kernel Core ⚙️

- Kernel Entry
- Early initialization
- Zeroing BSS
- Panic & Debug

Build note: A small example kernel is available under `kernel/` and can be built with `build_kernel.bat` which compiles `kernel/kernel.c` and `kernel/kernel_entry.asm`, then links using `linker.ld` to produce `build/kernel.bin`.
  - Panic handler
  - Assertion system
  - Kernel logging
- CPU Abstraction
  - Register access
  - Privilege level control
  - Atomic operations

---

3. Interrupt & Exception Handling ⚠️

- Interrupt Descriptor Tables / Vector Tables
- IRQ Management
  - Masking / unmasking
  - IRQ dispatch
- Exception Handling
  - Page faults
  - Invalid instructions
  - Syscall traps
- Timer Interrupts
  - Tick generation
  - Scheduler trigger

---

4. Memory Management 🧠

- Physical Memory Manager
  - Frame allocator
  - Free list / bitmap
- Virtual Memory Manager
  - Page tables
  - Address translation
  - Memory protection
- Kernel Heap
  - kmalloc
  - kfree
- User Memory
  - Process address space
  - User heap management

---

5. Process Management 👥

- Process Control Block (PCB)
- Process Lifecycle
  - Create
  - Run
  - Block
  - Exit
- Context Switching
  - Register save/restore
  - Stack switching
- Scheduler
  - Ready queue
  - Round-robin policy
  - Preemption support

---

6. Syscall Interface 🔁

- Syscall ABI
  - Register-based arguments
  - Return conventions
- Syscall Dispatcher
- Core Syscalls
  - read
  - write
  - exit
  - fork (optional)
  - exec
- User ↔ Kernel Boundary Enforcement

---

7. Executable Loader 📦

- ELF Parser
  - Headers
  - Program segments
- Loader
  - Memory mapping
  - Stack setup
  - Entry point transfer
- Dynamic Linking (optional future)

---

8. C / C++ Runtime 🧩

- Minimal libc
  - memcpy, memset
  - malloc, free
  - printf
- C++ Runtime
  - Global constructors
  - new / delete
- ABI Compatibility Layer

---

9. Device Driver Framework 🧰

- Driver Model
  - Initialization hooks
  - Interrupt registration
- Bus Support
  - PCI (x86)
  - USB (later)
- Character Devices
- Block Devices

---

10. Input Devices ⌨️

- Keyboard Driver
  - Scan code handling
  - Key mapping
- Mouse Driver
  - Movement tracking
  - Button states
- Input Event Queue
  - Event buffering
  - Dispatch to UI

---

11. Graphics Subsystem 🖼️

- Framebuffer Driver
- Graphics Primitives
  - Pixel drawing
  - Rectangles
  - Text rendering
- Cursor Rendering

---

12. Terminal System 🖥️

- Text Console
- Terminal Emulator
- Line discipline
- Scrollback buffer
- Shell Interface
  - Command parsing
  - Process launching


---

13. Filesystem (Optional / Later) 💾

- VFS Layer
- In-memory FS
- Block Device Interface
- SD Card Driver (Pi)

---
---

14. GUI System 🪟

- Window Manager (minimal)
  - Window creation
  - Z-order
- Event System
  - Mouse events
  - Keyboard events
- Widgets
  - Button
  - Label
- Rendering Pipeline
  - Software rendering
  - Redraw regions


---

15. Networking Stack 🌐

- Network Device Drivers
  - Ethernet
  - USB NIC
- Network Core
  - Packet buffers
  - Routing
- Protocol Layers
  - ARP
  - IP
  - UDP
  - TCP (optional)
- Socket API
  - socket
  - send
  - recv
  - bind
