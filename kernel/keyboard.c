#include "keyboard.h"
#include "io.h"
#include "serial.h"

static const char scancode_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
};

char keyboard_read_char(void) {
    int c = serial_read_char();
    if (c >= 0) {
        if (c == '\r') return '\n';
        return (char)c;
    }

    if ((inb(0x64) & 1) == 0) return 0;

    uint8_t sc = inb(0x60);
    if (sc & 0x80) return 0;
    if (sc < sizeof(scancode_map)) return scancode_map[sc];
    return 0;
}
