#include "pic.h"

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pic_remap(uint8_t offset1, uint8_t offset2) {
    // ICW1 - Initialization
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2 - Vector Offsets
    outb(0x21, offset1);
    outb(0xA1, offset2);

    // ICW3 - Master/Slave Wiring
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4 - 8086 Mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
}

void pic_disable(void) {
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}