#ifndef PIC_H
#define PIC_H

#include "kernel.h"

// Remap PIC to offsets (typically 0x20 and 0x28)
void pic_remap(uint8_t offset1, uint8_t offset2);

// Mask all hardware interrupts
void pic_disable(void);

#endif
