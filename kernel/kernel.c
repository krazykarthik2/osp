#include "terminal.h"
#include "serial.h"
#include "shell.h"
#include "mmu.h"
#include "sched.h"
#include "proc.h"
#include "ext4.h"

void kmain(void) {
    serial_init();
    terminal_init();
    serial_write("OSP x86_64 Starting...\n");
    mmu_report();
    cache_report();

    sched_init(2);
    proc_init();
    serial_write("Managers ready\n\n");

    ext4_mount();
    
    shell_run();

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
