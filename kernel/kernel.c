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

    terminal_writeln("OSP x86_64 Starting...");
    mmu_report();
    cache_report();

    sched_init(2);
    proc_init();
    terminal_writeln("Managers ready");

    ext4_mount();
    
    terminal_writeln("--- Verifying shell and EXT4 ---");
    shell_exec("echo \"hello osp\" > test.txt");
    shell_exec("cat test.txt");
    shell_exec("echo \" world\" >> test.txt");
    shell_exec("cat test.txt");
    shell_exec("ls | echo");
    shell_exec("echo \"ls && echo done\" > script && cat script");
    terminal_writeln("--- Verification complete ---");

    shell_run();

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
