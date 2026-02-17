#include "shell.h"
#include "keyboard.h"
#include "terminal.h"
#include "mmu.h"
#include "io.h"

static int streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static int starts_with(const char* s, const char* prefix) {
    while (*prefix) {
        if (*s++ != *prefix++) return 0;
    }
    return 1;
}

static void shell_help(void) {
    terminal_writeln("Commands:");
    terminal_writeln("  help         - show commands");
    terminal_writeln("  clear        - clear screen");
    terminal_writeln("  echo <text>  - print text");
    terminal_writeln("  mmu          - paging status");
    terminal_writeln("  cache        - cache status");
    terminal_writeln("  reboot       - reboot machine");
}

static void reboot(void) {
    outb(0x64, 0xFE);
}

void shell_run(void) {
    char line[128];
    terminal_writeln("Simple shell ready. Type 'help'.");

    for (;;) {
        terminal_write("osp> ");
        size_t n = 0;

        for (;;) {
            char c = keyboard_read_char();
            if (!c) continue;

            if (c == '\n') {
                terminal_putc('\n');
                line[n] = 0;
                break;
            }

            if (c == '\b') {
                if (n > 0) {
                    n--;
                    terminal_putc('\b');
                }
                continue;
            }

            if (n < sizeof(line) - 1 && c >= 32 && c < 127) {
                line[n++] = c;
                terminal_putc(c);
            }
        }

        if (line[0] == 0) continue;
        if (streq(line, "help")) {
            shell_help();
        } else if (streq(line, "clear")) {
            terminal_clear();
        } else if (starts_with(line, "echo ")) {
            terminal_writeln(line + 5);
        } else if (streq(line, "mmu")) {
            mmu_report();
        } else if (streq(line, "cache")) {
            cache_report();
        } else if (streq(line, "reboot")) {
            terminal_writeln("Rebooting...");
            reboot();
        } else {
            terminal_writeln("Unknown command. Try: help");
        }
    }
}
