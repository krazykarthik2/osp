#ifndef PIC_H
#define PIC_H

#include <stdint.h>

// Remap PIC to offsets (typically 0x20 and 0x28)
void pic_remap(uint8_t offset1, uint8_t offset2);

// Mask all hardware interrupts
void pic_disable(void);

// Individual port I/O if needed elsewhere
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

#endif