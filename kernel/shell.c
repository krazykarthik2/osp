#include "../include/shell.h"
#include "../include/console.h"
#include "../include/kbd.h"
#include "../include/sched.h"

static int streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == 0 && *b == 0;
}

static void read_line(char* buf, int cap) {
    int n = 0;
    for (;;) {
        char c = kbd_getchar();
        if (c == '\n') {
            console_putc('\n');
            buf[n] = 0;
            return;
        }
        if (c == '\b') {
            if (n > 0) {
                n--;
                console_putc('\b');
            }
            continue;
        }
        if (n < cap - 1) {
            buf[n++] = c;
            console_putc(c);
        }
        sched_yield();
    }
}

void shell_task(void) {
    console_writeln("simple shell: type `help`");
    for (;;) {
        char line[64];
        console_write("> ");
        read_line(line, (int)sizeof(line));

        if (streq(line, "help")) {
            console_writeln("commands: help, ps, yield, clear");
        } else if (streq(line, "ps")) {
            sched_dump_tasks();
        } else if (streq(line, "yield")) {
            sched_yield();
        } else if (streq(line, "clear")) {
            console_clear(0x0F);
        } else if (line[0] == 0) {
            // no-op
        } else {
            console_write("unknown: ");
            console_writeln(line);
        }

        sched_yield();
    }
}
