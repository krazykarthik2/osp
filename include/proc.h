#ifndef PROC_H
#define PROC_H

#include "common.h"

void proc_init(void);
int proc_spawn_bg(const char* name, uint32_t work_ticks);
int proc_kill(int pid);
void proc_ps(void);
void proc_top(void);

#endif
