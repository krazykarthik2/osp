#include "shell.h"
#include "keyboard.h"
#include "terminal.h"
#include "mmu.h"
#include "sched.h"
#include "proc.h"
#include "ext4.h"
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

static char token_storage[2048];
static int token_storage_ptr = 0;

static char* next_token(char** input) {
    char* s = skip_spaces(*input);
    if (*s == 0) return 0;
    
    char* tok_start = &token_storage[token_storage_ptr];
    
    if (*s == '"') {
        s++;
        while (*s && *s != '"') {
            token_storage[token_storage_ptr++] = *s++;
        }
        if (*s == '"') s++;
    } else if ((*s == '>' && *(s+1) == '>') || (*s == '&' && *(s+1) == '&') || (*s == '|' && *(s+1) == '|')) {
        token_storage[token_storage_ptr++] = *s++;
        token_storage[token_storage_ptr++] = *s++;
    } else if (*s == '>' || *s == '|' || *s == '&' || *s == ';') {
        token_storage[token_storage_ptr++] = *s++;
    } else {
        while (*s && *s != ' ' && *s != '>' && *s != '|' && *s != '&' && *s != ';') {
            token_storage[token_storage_ptr++] = *s++;
        }
    }
    token_storage[token_storage_ptr++] = 0;
    *input = s;
    return tok_start;
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
static int cmd_ls(int argc, char** argv);
static int cmd_cd(int argc, char** argv);
static int cmd_mkdir(int argc, char** argv);
static int cmd_touch(int argc, char** argv);
static int cmd_cat(int argc, char** argv);
static int cmd_rm(int argc, char** argv);
static int cmd_tree(int argc, char** argv);
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
    { "ls", "ls [-r]                      - list files", cmd_ls },
    { "tree", "tree                         - tree view of files", cmd_tree },
    { "cd", "cd <dir>                     - change directory", cmd_cd },
    { "mkdir", "mkdir <dir>                  - create directory", cmd_mkdir },
    { "touch", "touch <file> <content>       - create file", cmd_touch },
    { "cat", "cat <file>                   - display file content", cmd_cat },
    { "rm", "rm <file>                    - remove file/directory", cmd_rm },
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

static int cmd_ls(int argc, char** argv) {
    if (argc >= 2 && streq(argv[1], "-r")) {
        ext4_tree();
        return 0;
    }
    ext4_ls();
    return 0;
}

static int cmd_tree(int argc, char** argv) {
    (void)argc; (void)argv;
    ext4_tree();
    return 0;
}

static int cmd_cd(int argc, char** argv) {
    if (argc < 2) return 0;
    if (ext4_cd(argv[1]) != 0) {
        terminal_writeln("No such directory");
    }
    return 0;
}

static int cmd_mkdir(int argc, char** argv) {
    if (argc < 2) {
        terminal_writeln("Usage: mkdir <name>");
        return -1;
    }
    ext4_mkdir(argv[1]);
    return 0;
}

static int cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        terminal_writeln("Usage: touch <name> [content]");
        return -1;
    }
    const char* content = "";
    if (argc >= 3) content = argv[2];
    ext4_touch(argv[1], content);
    return 0;
}

static int cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        terminal_writeln("Usage: cat <name>");
        return -1;
    }
    ext4_cat(argv[1]);
    return 0;
}

static int cmd_rm(int argc, char** argv) {
    if (argc < 2) {
        terminal_writeln("Usage: rm <file>");
        return -1;
    }
    ext4_rm(argv[1]);
    return 0;
}

static int cmd_reboot(int argc, char** argv) {
    (void)argc;
    (void)argv;
    terminal_writeln("Rebooting...");
    reboot();
    return 0;
}

static char shell_capture_buf[1024];
static int shell_capture_idx = 0;
static char shell_pipe_input[1024];

static void shell_capture_putc(char c) {
    if (shell_capture_idx < 1023) {
        shell_capture_buf[shell_capture_idx++] = c;
        shell_capture_buf[shell_capture_idx] = 0;
    }
    terminal_putc_direct(c);
}

static int dispatch_command(int argc, char** argv) {
    if (argc == 0) return 0;
    
    // Check for redirection/operators in the arguments
    char* redir_out = 0;
    int append = 0;
    int actual_argc = 0;
    int i;
    for (i = 0; i < argc; i++) {
        if (streq(argv[i], ">") || streq(argv[i], ">>")) {
            append = streq(argv[i], ">>");
            if (i + 1 < argc) {
                redir_out = argv[i+1];
                break;
            }
        }
        actual_argc++;
    }

    if (redir_out) {
        shell_capture_idx = 0;
        shell_capture_buf[0] = 0;
        terminal_set_custom_putc(shell_capture_putc);
    }

    int ret = -1;
    for (i = 0; i < cli_command_count(); i++) {
        if (streq(argv[0], cli_commands[i].name)) {
            ret = cli_commands[i].handler(actual_argc, argv);
            break;
        }
    }

    if (redir_out) {
        terminal_set_custom_putc(0);
        ext4_write(redir_out, shell_capture_buf, append);
    }

    if (ret == -1 && argc > 0) {
        terminal_writeln("Unknown command. Type: help");
    }
    return ret;
}

void shell_exec(const char* command_line) {
    char line[1024];
    int len = 0;
    while (command_line[len] && len < 1023) {
        line[len] = command_line[len];
        len++;
    }
    line[len] = 0;

    char* cursor = line;
    char* argv[16];
    int last_status = 0;
    int skip_remaining = 0;
    int piped_output_active = 0;
    token_storage_ptr = 0;

    while (*cursor && !skip_remaining) {
        int argc = 0;
        char* op = 0;
        while (argc < 15) {
            char* tok = next_token(&cursor);
            if (!tok) break;
            if (streq(tok, "&&") || streq(tok, "|") || streq(tok, "||") || streq(tok, ";")) {
                op = tok;
                break;
            }
            argv[argc++] = tok;
        }
        argv[argc] = 0;

        if (argc > 0) {
            if (piped_output_active) {
                int p = 0; while(shell_capture_buf[p]) { shell_pipe_input[p] = shell_capture_buf[p]; p++; }
                shell_pipe_input[p] = 0;
                if (argc < 15) {
                    argv[argc++] = shell_pipe_input;
                    argv[argc] = 0;
                }
                piped_output_active = 0;
            }

            if (op && streq(op, "|")) {
                shell_capture_idx = 0;
                shell_capture_buf[0] = 0;
                terminal_set_custom_putc(shell_capture_putc);
                last_status = dispatch_command(argc, argv);
                terminal_set_custom_putc(0);
                piped_output_active = 1;
            } else {
                last_status = dispatch_command(argc, argv);
            }
        }
        if (op) {
            if (streq(op, "&&")) {
                if (last_status != 0) skip_remaining = 1;
            } else if (streq(op, "||")) {
                if (last_status == 0) skip_remaining = 1;
            }
        }
    }
}

void shell_run(void) {
    char line[1024];
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
        shell_exec(line);
    }
}
