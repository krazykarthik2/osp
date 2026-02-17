#include "shell.h"
#include "keyboard.h"
#include "terminal.h"
#include "mmu.h"
#include "sched.h"
#include "proc.h"
#include "io.h"

static void term_write_u32(uint32_t value) {
    char buf[16];
    int i = 0;
    if (value == 0) {
        terminal_putc('0');
        return;
    }
    while (value > 0 && i < (int)sizeof(buf)) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (i-- > 0) terminal_putc(buf[i]);
}

static int streq(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static uint32_t parse_u32(const char* s) {
    uint32_t v = 0;
    while (*s >= '0' && *s <= '9') {
        v = v * 10u + (uint32_t)(*s - '0');
        s++;
    }
    return v;
}

static char* skip_spaces(char* s) {
    while (*s == ' ') s++;
    return s;
}

static char* next_token(char** input) {
    char* s = skip_spaces(*input);
    if (*s == 0) {
        *input = s;
        return 0;
    }
    char* start = s;
    while (*s && *s != ' ') s++;
    if (*s) {
        *s = 0;
        s++;
    }
    *input = s;
    return start;
}

typedef int (*cmd_handler_t)(int argc, char** argv);

typedef struct {
    const char* name;
    const char* usage;
    cmd_handler_t handler;
} cli_cmd_t;

static int cmd_help(int argc, char** argv);
static int cmd_clear(int argc, char** argv);
static int cmd_echo(int argc, char** argv);
static int cmd_mmu(int argc, char** argv);
static int cmd_cache(int argc, char** argv);
static int cmd_sched(int argc, char** argv);
static int cmd_ps(int argc, char** argv);
static int cmd_top(int argc, char** argv);
static int cmd_bg(int argc, char** argv);
static int cmd_kill(int argc, char** argv);
static int cmd_uname(int argc, char** argv);
static int cmd_uptime(int argc, char** argv);
static int cmd_reboot(int argc, char** argv);

static const cli_cmd_t cli_commands[] = {
    { "help", "help                         - show command list", cmd_help },
    { "clear", "clear                        - clear terminal", cmd_clear },
    { "echo", "echo <text>                  - print text", cmd_echo },
    { "mmu", "mmu                          - paging status", cmd_mmu },
    { "cache", "cache                        - cache status", cmd_cache },
    { "sched", "sched                        - scheduler stats", cmd_sched },
    { "ps", "ps                           - list processes", cmd_ps },
    { "top", "top                          - cpu/process usage snapshot", cmd_top },
    { "bg", "bg <name> [ticks]            - start background process", cmd_bg },
    { "kill", "kill <pid>                   - terminate process", cmd_kill },
    { "uname", "uname                        - system name", cmd_uname },
    { "uptime", "uptime                       - scheduler tick uptime", cmd_uptime },
    { "reboot", "reboot                       - reboot machine", cmd_reboot },
};

static int cli_command_count(void) {
    return (int)(sizeof(cli_commands) / sizeof(cli_commands[0]));
}

static void reboot(void) {
    outb(0x64, 0xFE);
}

static int cmd_help(int argc, char** argv) {
    (void)argc;
    (void)argv;
    terminal_writeln("OSP CLI commands:");
    for (int i = 0; i < cli_command_count(); i++) {
        terminal_write("  ");
        terminal_writeln(cli_commands[i].usage);
    }
    return 0;
}

static int cmd_clear(int argc, char** argv) {
    (void)argc;
    (void)argv;
    terminal_clear();
    return 0;
}

static int cmd_echo(int argc, char** argv) {
    if (argc < 2) {
        terminal_writeln("");
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        terminal_write(argv[i]);
        if (i + 1 < argc) terminal_putc(' ');
    }
    terminal_putc('\n');
    return 0;
}

static int cmd_mmu(int argc, char** argv) {
    (void)argc;
    (void)argv;
    mmu_report();
    return 0;
}

static int cmd_cache(int argc, char** argv) {
    (void)argc;
    (void)argv;
    cache_report();
    return 0;
}

static int cmd_sched(int argc, char** argv) {
    (void)argc;
    (void)argv;
    sched_dump();
    return 0;
}

static int cmd_ps(int argc, char** argv) {
    (void)argc;
    (void)argv;
    proc_ps();
    return 0;
}

static int cmd_top(int argc, char** argv) {
    (void)argc;
    (void)argv;
    proc_top();
    return 0;
}

static int cmd_bg(int argc, char** argv) {
    if (argc < 2) {
        terminal_writeln("Usage: bg <name> [ticks]");
        return -1;
    }
    uint32_t ticks = 200;
    if (argc >= 3) ticks = parse_u32(argv[2]);
    if (ticks == 0) ticks = 1;

    int pid = proc_spawn_bg(argv[1], ticks);
    if (pid < 0) {
        terminal_writeln("Failed to spawn background process");
        return -1;
    }

    terminal_write("Started pid ");
    term_write_u32((uint32_t)pid);
    terminal_putc('\n');
    return 0;
}

static int cmd_kill(int argc, char** argv) {
    if (argc < 2) {
        terminal_writeln("Usage: kill <pid>");
        return -1;
    }
    uint32_t pid = parse_u32(argv[1]);
    if (pid == 0) {
        terminal_writeln("Usage: kill <pid>");
        return -1;
    }
    if (proc_kill((int)pid) == 0) terminal_writeln("Process terminated");
    else terminal_writeln("No such pid");
    return 0;
}

static int cmd_uname(int argc, char** argv) {
    (void)argc;
    (void)argv;
    terminal_writeln("OSP x86_64 kernel");
    return 0;
}

static int cmd_uptime(int argc, char** argv) {
    (void)argc;
    (void)argv;
    terminal_write("ticks: ");
    term_write_u32(sched_get_tick_count());
    terminal_putc('\n');
    return 0;
}

static int cmd_reboot(int argc, char** argv) {
    (void)argc;
    (void)argv;
    terminal_writeln("Rebooting...");
    reboot();
    return 0;
}

static int dispatch_command(int argc, char** argv) {
    if (argc == 0) return 0;
    for (int i = 0; i < cli_command_count(); i++) {
        if (streq(argv[0], cli_commands[i].name)) {
            return cli_commands[i].handler(argc, argv);
        }
    }
    terminal_writeln("Unknown command. Type: help");
    return -1;
}

void shell_run(void) {
    char line[128];
    char* argv[12];
    terminal_writeln("OSP CLI ready. Type 'help'.");

    for (;;) {
        terminal_write("osp@kernel:$ ");
        size_t n = 0;

        for (;;) {
            char c = keyboard_read_char();
            if (!c) {
                sched_run_ticks(1);
                continue;
            }

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

        char* cursor = line;
        int argc = 0;
        while (argc < (int)(sizeof(argv) / sizeof(argv[0]))) {
            char* tok = next_token(&cursor);
            if (!tok) break;
            argv[argc++] = tok;
        }

        dispatch_command(argc, argv);
    }
}
