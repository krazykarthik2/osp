#ifndef PANIC_H
#define PANIC_H

#include "kernel.h"

void kpanic(const char* msg, uint32_t a, uint32_t b);

#endif
