#include "shell.h"
#include "terminal.h"
#include "serial.h"
#include "ext4.h"
#include "keyboard.h"

#define MAX_TOKENS 32
typedef struct {
    char* text; 
    int type; // 0=word, 1=>, 2=>>, 3=|, 4=&&, 5=||
} shell_token_t;

static shell_token_t tokens[MAX_TOKENS];
static char* shell_capture_buf = (char*)0x510000;
static int shell_capture_idx = 0;

static int streq(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *a == *b;
}

static void shell_capture_putc(char c) {
    if(shell_capture_idx < 4095) {
        shell_capture_buf[shell_capture_idx++] = c;
        shell_capture_buf[shell_capture_idx] = 0;
    }
}

static char token_storage[512];
static int storage_idx = 0;

static void dispatch_command(int start, int end) {
    if(start >= end) return;
    char* cmd = tokens[start].text;
    int redir_type = 0; char* redir_file = 0;
    int redir_idx = -1;
    int pipe_to = -1;
    
    for(int i=start; i<end; i++) {
        if(tokens[i].type == 1 || tokens[i].type == 2) { 
            redir_type = tokens[i].type; 
            if(i + 1 < end) {
                redir_file = tokens[i+1].text; 
                redir_idx = i;
            }
            break; 
        }
        if(tokens[i].type == 3) { pipe_to = i + 1; break; }
    }

    if(redir_type || pipe_to != -1) {
        shell_capture_idx = 0; shell_capture_buf[0] = 0;
        terminal_set_custom_putc(shell_capture_putc);
    }

    int cmd_end = (redir_idx != -1) ? redir_idx : ((pipe_to != -1) ? pipe_to - 1 : end);

    int found = 0;
    if(streq(cmd, "ls")) { ext4_ls(); found = 1; }
    else if(streq(cmd, "cat")) { 
        found = 1;
        if(start+1 < cmd_end) ext4_cat(tokens[start+1].text);
        else terminal_write("cat: missing file\n");
    }
    else if(streq(cmd, "clear")) { terminal_clear(); found = 1; }
    else if(streq(cmd, "help")) {
        terminal_write("Commands: ls, cat, echo, clear, help\n");
        found = 1;
    }
    else if(streq(cmd, "echo")) {
        found = 1;
        for(int i=start+1; i<cmd_end; i++) {
            terminal_write(tokens[i].text);
            if(i + 1 < cmd_end) terminal_write(" ");
        }
        terminal_write("\n");
    }
    
    if(!found) {
        terminal_write("Unknown command: ");
        terminal_write(cmd);
        terminal_write("\n");
    }

    if(redir_type || pipe_to != -1) {
        terminal_set_custom_putc(0);
        if(redir_type == 1 && redir_file) {
            ext4_write(redir_file, shell_capture_buf, 0);
        }
        else if(redir_type == 2 && redir_file) ext4_write(redir_file, shell_capture_buf, 1);
        else if(pipe_to != -1) {
            dispatch_command(pipe_to, end); 
        }
    }
}

void shell_exec(const char* input) {
    if(!input || !*input) return;
    storage_idx = 0;
    int token_count = 0;
    const char* p = input;
    
    while(*p && token_count < MAX_TOKENS) {
        while(*p == ' ') p++; 
        if(!*p) break;

        if (storage_idx >= 500) break;
        tokens[token_count].text = &token_storage[storage_idx];
        
        if(*p == '>') {
            if(*(p+1) == '>') { tokens[token_count].type = 2; token_storage[storage_idx++] = '>'; token_storage[storage_idx++] = '>'; p+=2; }
            else { tokens[token_count].type = 1; token_storage[storage_idx++] = '>'; p++; }
        } else if(*p == '|') {
            tokens[token_count].type = 3; token_storage[storage_idx++] = '|'; p++;
        } else {
            tokens[token_count].type = 0;
            while(*p && *p != ' ' && *p != '>' && *p != '|') {
                if (storage_idx < 500) token_storage[storage_idx++] = *p++;
                else break;
            }
        }
        if (storage_idx < 511) token_storage[storage_idx++] = 0;
        token_count++;
    }
    dispatch_command(0, token_count);
}

void shell_run(void) {
    static char cmd[256];
    int cmd_i = 0;

    serial_write("Shell ready.\n");
    terminal_write("osp> ");

    while (1) {
        char c = keyboard_read_char();
        if (c > 0) {
            if (c == '\n') {
                cmd[cmd_i] = 0;
                terminal_putc('\n');
                if (cmd_i > 0) {
                    shell_exec(cmd);
                }
                cmd_i = 0;
                terminal_write("osp> ");
            } else if (c == '\b') {
                if (cmd_i > 0) {
                    cmd_i--;
                    terminal_putc('\b');
                }
            } else if (cmd_i < 255) {
                cmd[cmd_i++] = c;
                terminal_putc(c);
            }
        }
        // Small delay to prevent CPU hogging
        for(volatile int i=0; i<10000; i++);
    }
}
