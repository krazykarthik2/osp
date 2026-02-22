#include "keyboard.h"
#include "io.h"
#include "serial.h"

static int lshift_down = 0;
static int rshift_down = 0;
static int capslock_active = 0;

static const char scancode_map[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8',
    '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0,
};

static const char shifted_scancode_map[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8',
    '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0,
};

char keyboard_read_char(void) {
    int sc_ser = serial_read_char();
    if (sc_ser >= 0) {
        if(sc_ser < 32 || sc_ser > 126) {
           serial_write("[S:"); 
           serial_putc(((sc_ser/100)%10)+'0');
           serial_putc(((sc_ser/10)%10)+'0');
           serial_putc(((sc_ser)%10)+'0');
           serial_write("]");
        }
        if (sc_ser == '\r' || sc_ser == '\n') return '\n';
        return (char)sc_ser;
    }

    if ((inb(0x64) & 1) == 0) return 0;

    uint8_t sc = inb(0x60);
    
    // Check for shift keys
    if (sc == 0x2A) { lshift_down = 1; return 0; }
    if (sc == 0xAA) { lshift_down = 0; return 0; }
    if (sc == 0x36) { rshift_down = 1; return 0; }
    if (sc == 0xB6) { rshift_down = 0; return 0; }
    
    // Check for Caps Lock
    if (sc == 0x3A) { capslock_active = !capslock_active; return 0; }

    // Ignore other release events
    if (sc & 0x80) return 0;

    if (sc < 128) {
        int shift_active = (lshift_down || rshift_down);
        char c = scancode_map[sc];
        
        // Handle Caps Lock vs Shift logic for alpha characters
        if (c >= 'a' && c <= 'z') {
            if (shift_active ^ capslock_active) {
                return shifted_scancode_map[sc];
            } else {
                return c;
            }
        }
        
        // For non-alpha keys, only Shift matters
        if (shift_active) {
            return shifted_scancode_map[sc];
        } else {
            return c;
        }
    }
    return 0;
}
