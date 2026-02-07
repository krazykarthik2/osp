#include "../include/console.h"

static volatile uint16_t* const vga = (volatile uint16_t*)0xB8000;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t current_color = 0x0F;

static void put_at(char c, uint8_t x, uint8_t y, uint8_t color) {
    vga[(uint16_t)y * 80u + (uint16_t)x] = ((uint16_t)color << 8) | (uint8_t)c;
}

static void scroll_if_needed(void) {
    if (cursor_y < 25) return;

    for (uint16_t y = 1; y < 25; y++) {
        for (uint16_t x = 0; x < 80; x++) {
            vga[(y - 1) * 80 + x] = vga[y * 80 + x];
        }
    }
    for (uint16_t x = 0; x < 80; x++) {
        vga[24 * 80 + x] = ((uint16_t)current_color << 8) | 0x20;
    }
    cursor_y = 24;
}

void console_clear(uint8_t color) {
    current_color = color;
    for (uint16_t i = 0; i < 80u * 25u; i++) {
        vga[i] = ((uint16_t)color << 8) | 0x20;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void console_putc(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        scroll_if_needed();
        return;
    }
    if (c == '\r') {
        cursor_x = 0;
        return;
    }
    if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
        put_at(' ', cursor_x, cursor_y, current_color);
        return;
    }

    put_at(c, cursor_x, cursor_y, current_color);
    cursor_x++;
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
        scroll_if_needed();
    }
}

void console_write(const char* s) {
    for (; *s; s++) console_putc(*s);
}

void console_writeln(const char* s) {
    console_write(s);
    console_putc('\n');
}

void console_write_hex(uint32_t v) {
    static const char* hex = "0123456789ABCDEF";
    console_write("0x");
    for (int i = 28; i >= 0; i -= 4) {
        console_putc(hex[(v >> (uint32_t)i) & 0xF]);
    }
}
