#ifndef SCHED_H
#define SCHED_H

#include "kernel.h"

typedef void (*task_entry_t)(void);

void sched_init(void);
int task_spawn(const char* name, task_entry_t entry);
void sched_start(void);
void sched_yield(void);

void sched_dump_tasks(void);

#endif
