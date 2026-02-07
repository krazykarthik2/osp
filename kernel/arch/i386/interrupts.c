#include "../../../include/kernel.h"
#include "../../../include/panic.h"

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
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

static void idt_set_gate_phys(struct idt_entry* table, uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    table[num].base_lo = base & 0xFFFF;
    table[num].base_hi = (base >> 16) & 0xFFFF;
    table[num].sel     = sel;
    table[num].always0 = 0;
    table[num].flags   = flags;
}

void idt_install() {
    // 1. Physical load address of kernel in RAM
    uint32_t load_addr = KERNEL_PHYS_BASE;

    struct idt_entry* idt_phys = (struct idt_entry*)((uint32_t)idt + load_addr);
    struct idt_ptr* idtp_phys = (struct idt_ptr*)((uint32_t)&idtp + load_addr);

    idtp_phys->limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp_phys->base  = (uint32_t)idt_phys;

    // 3. Clear the IDT
    for(int i = 0; i < 256; i++) {
        idt_set_gate_phys(idt_phys, i, 0, 0, 0);
    }

    // 4. Install exception handlers (0-31)
    idt_set_gate_phys(idt_phys, 0,  (uint32_t)isr0  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 1,  (uint32_t)isr1  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 2,  (uint32_t)isr2  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 3,  (uint32_t)isr3  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 4,  (uint32_t)isr4  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 5,  (uint32_t)isr5  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 6,  (uint32_t)isr6  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 7,  (uint32_t)isr7  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 8,  (uint32_t)isr8  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 9,  (uint32_t)isr9  + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 10, (uint32_t)isr10 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 11, (uint32_t)isr11 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 12, (uint32_t)isr12 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 13, (uint32_t)isr13 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 14, (uint32_t)isr14 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 15, (uint32_t)isr15 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 16, (uint32_t)isr16 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 17, (uint32_t)isr17 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 18, (uint32_t)isr18 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 19, (uint32_t)isr19 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 20, (uint32_t)isr20 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 21, (uint32_t)isr21 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 22, (uint32_t)isr22 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 23, (uint32_t)isr23 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 24, (uint32_t)isr24 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 25, (uint32_t)isr25 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 26, (uint32_t)isr26 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 27, (uint32_t)isr27 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 28, (uint32_t)isr28 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 29, (uint32_t)isr29 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 30, (uint32_t)isr30 + load_addr, 0x08, 0x8E);
    idt_set_gate_phys(idt_phys, 31, (uint32_t)isr31 + load_addr, 0x08, 0x8E);


    // 5. Load IDT using relocated pointer to the pointer struct
    __asm__ __volatile__("lidt (%0)" : : "r"(idtp_phys));
}

void isr_handler_c(uint32_t vec, uint32_t err, uint32_t* regs) {
    (void)regs;
    kpanic("EXCEPTION", vec, err);
}
