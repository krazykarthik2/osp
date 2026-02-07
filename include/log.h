#ifndef LOG_H
#define LOG_H

#include "kernel.h"

void klog(const char* s);
void klog_hex(uint32_t v);
void klog_ln(const char* s);

#endif
