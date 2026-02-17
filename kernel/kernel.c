#include "terminal.h"
#include "serial.h"
#include "shell.h"
#include "mmu.h"

void kmain(void) {
    serial_init();
    terminal_init();

    terminal_writeln("OSP x86_64");
    terminal_writeln("Minimal non-GUI terminal OS running on QEMU/GRUB");
    mmu_report();
    cache_report();

    shell_run();

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
