#ifndef KERNEL_H
#define KERNEL_H

// Basic types to make life easier
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

// Relocation offset (linker already places the kernel at 0x10000)
#define KERNEL_PHYS_BASE 0x0u

void kernel_main(void);

#endif
