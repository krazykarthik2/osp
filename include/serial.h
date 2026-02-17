#ifndef SERIAL_H
#define SERIAL_H

#include "common.h"

void serial_init(void);
void serial_write(const char* s);
void serial_putc(char c);
int serial_read_char(void);

#endif
