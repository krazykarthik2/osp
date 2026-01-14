#include "../../../include/kernel.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

// Initialize to force into .data section so it exists in the binary
struct gdt_entry gdt[3] = { {0}, {0}, {0} };

extern void gdt_flush(uint32_t);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran, uint32_t load_addr) {
    // Calculate ACTUAL physical address of the entry
    struct gdt_entry* target = (struct gdt_entry*)((uint32_t)&gdt[num] + load_addr);

    target->base_low    = (base & 0xFFFF);
    target->base_middle = (base >> 16) & 0xFF;
    target->base_high   = (base >> 24) & 0xFF;
    target->limit_low   = (limit & 0xFFFF);
    target->granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    target->access      = access;
}

void gdt_install() {
    uint32_t load_addr;
    // Get physical offset of the kernel in RAM
    __asm__ ("call 1f; 1: pop %0; sub $1b, %0" : "=r"(load_addr));

    // 1. Setup Gates at their physical RAM locations
    gdt_set_gate(0, 0, 0, 0, 0, load_addr);                // Null
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF, load_addr);    // Code (0x08)
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF, load_addr);    // Data (0x10)

    // 2. Calculate physical addresses for the call
    uint32_t gdt_array_phys = ((uint32_t)&gdt) + load_addr;
    uint32_t flush_phys = ((uint32_t)gdt_flush) + load_addr;

    // 3. Call gdt_flush physically
    void (*physical_gdt_flush)(uint32_t) = (void (*)(uint32_t))flush_phys;
    
    // We pass the address of the ARRAY, not a pointer struct
    physical_gdt_flush(gdt_array_phys);
}