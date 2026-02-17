#include "terminal.h"
#include "serial.h"
#include "shell.h"
#include "mmu.h"
#include "sched.h"
#include "proc.h"

void kmain(void) {
    serial_init();
    terminal_init();

    terminal_writeln("OSP x86_64");
    terminal_writeln("Minimal non-GUI terminal OS running on QEMU/GRUB");
    mmu_report();
    terminal_writeln("MMU report done");
    cache_report();
    terminal_writeln("Cache report done");

    sched_init(2);
    proc_init();
    terminal_writeln("Process manager ready");

    shell_run();

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
