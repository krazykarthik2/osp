#ifndef TERMINAL_H
#define TERMINAL_H

#include "common.h"

void terminal_init(void);
void terminal_clear(void);
void terminal_write(const char* s);
void terminal_writeln(const char* s);
void terminal_putc(char c);
void terminal_putc_direct(char c);
void terminal_set_custom_putc(void (*f)(char));

#endif
