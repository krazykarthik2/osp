#ifndef CONSOLE_H
#define CONSOLE_H

#include "kernel.h"

void console_clear(uint8_t color);
void console_putc(char c);
void console_write(const char* s);
void console_writeln(const char* s);
void console_write_hex(uint32_t v);

#endif
