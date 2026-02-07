#include "../include/panic.h"
#include "../include/log.h"

void kpanic(const char* msg, uint32_t a, uint32_t b) {
    __asm__ __volatile__("cli");
    klog("\nPANIC: ");
    klog(msg);
    klog(" a=");
    klog_hex(a);
    klog(" b=");
    klog_hex(b);
    klog("\nSystem halted.\n");
    for (;;) __asm__ __volatile__("hlt");
}
