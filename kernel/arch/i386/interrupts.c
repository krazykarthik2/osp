#include "../../../include/kernel.h"
extern void clrscr_ext(unsigned char color);
struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry idt[256] __attribute__((aligned(16)));
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

void idt_install() {
    // 1. Calculate physical load address again
    uint32_t load_addr;
    __asm__ ("call 1f; 1: pop %0; sub $1b, %0" : "=r"(load_addr));

    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    
    // 2. Relocate the IDT base address
    idtp.base  = ((uint32_t)&idt) + load_addr;

    // 3. Clear the IDT
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    clrscr_ext(0x0F); // Clear screen to White on Black
    char msg[] = "IDT Installed";
    volatile char* vga = (volatile char*)0xB8000;
    int pos = 4;
    for (int i = 0; msg[i] != 0; i++) {
        vga[pos++] = msg[i];   // Write Character
        vga[pos++] = 0x0F;     // Write Attribute (White on Black)
    }


    // 4. Load IDT using relocated pointer to the pointer struct
    uint32_t idtp_phys = ((uint32_t)&idtp) + load_addr;
    __asm__ __volatile__("lidt (%0)" : : "r"(idtp_phys));
}