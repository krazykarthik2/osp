#include "terminal.h"
#include "serial.h"

static volatile uint16_t* const VGA = (uint16_t*)0xB8000;
static size_t row;
static size_t col;
static uint8_t color;
static void (*custom_putc_fn)(char) = 0;

void terminal_set_custom_putc(void (*f)(char)) {
    custom_putc_fn = f;
}

static void scroll(void) {
    for (size_t y = 1; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            VGA[(y - 1) * 80 + x] = VGA[y * 80 + x];
        }
    }
    for (size_t x = 0; x < 80; x++) {
        VGA[24 * 80 + x] = ((uint16_t)color << 8) | ' ';
    }
    row = 24;
    col = 0;
}

void terminal_init(void) {
    row = 0;
    col = 0;
    color = 0x0F;
    terminal_clear();
}

void terminal_clear(void) {
    for (size_t i = 0; i < 80 * 25; i++) {
        VGA[i] = ((uint16_t)color << 8) | ' ';
    }
    row = 0;
    col = 0;
}

void terminal_putc_direct(char c) {
    if (c == '\n') {
        col = 0;
        row++;
        serial_putc('\n');
    } else if (c == '\b') {
        if (col > 0) {
            col--;
            VGA[row * 80 + col] = ((uint16_t)color << 8) | ' ';
            serial_write("\b \b");
        }
    } else {
        VGA[row * 80 + col] = ((uint16_t)color << 8) | (uint8_t)c;
        col++;
        serial_putc(c);
    }

    if (col >= 80) {
        col = 0;
        row++;
    }
    if (row >= 25) {
        scroll();
    }
}

void terminal_putc(char c) {
    if (custom_putc_fn) {
        custom_putc_fn(c);
        return;
    }
    terminal_putc_direct(c);
}

void terminal_write(const char* s) {
    while (*s) terminal_putc(*s++);
}

void terminal_writeln(const char* s) {
    terminal_write(s);
    terminal_putc('\n');
}
