#include "../include/console.h"
#include "../include/pic.h"
#include "../include/sched.h"
#include "../include/shell.h"
extern void idt_install(void);

void clrscr_ext(unsigned char color) {
    console_clear((uint8_t)color);
}

static void background_counter_task(void) {
    uint32_t n = 0;
    for (;;) {
        if ((n % 500000u) == 0) {
            console_write("[bg] tick ");
            console_write_hex(n);
            console_putc('\n');
        }
        n++;
        if ((n % 20000u) == 0) sched_yield();
    }
}

void kernel_main(void) {
    console_clear(0x0F);
    console_writeln("kernel_main: protected mode OK");

    pic_disable();
    idt_install();

    sched_init();
    task_spawn("shell", shell_task);
    task_spawn("bg-counter", background_counter_task);

    sched_start();

    for (;;) __asm__ __volatile__("hlt");
}
