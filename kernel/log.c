#include "../include/log.h"
#include "../include/console.h"

void klog(const char* s) {
    console_write(s);
}

void klog_hex(uint32_t v) {
    console_write_hex(v);
}

void klog_ln(const char* s) {
    console_writeln(s);
}
