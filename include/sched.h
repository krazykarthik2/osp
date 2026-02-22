#ifndef SCHED_H
#define SCHED_H

#include "common.h"

typedef int (*kthread_step_t)(void* ctx);

enum {
	SCHED_THREAD_UNUSED = 0,
	SCHED_THREAD_READY = 1,
	SCHED_THREAD_RUNNING = 2,
	SCHED_THREAD_DONE = 3,
};

typedef struct {
	int tid;
	const char* name;
	uint32_t affinity_mask;
	uint8_t state;
	uint32_t run_count;
} sched_thread_info_t;

void sched_init(uint32_t cpu_count);
int sched_thread_create(const char* name, uint32_t affinity_mask, kthread_step_t step, void* ctx);
int sched_thread_kill(int tid);
void sched_run_ticks(uint32_t ticks);
uint32_t sched_get_tick_count(void);
uint32_t sched_get_cpu_count(void);
uint32_t sched_get_cpu_switches(uint32_t cpu);
uint32_t sched_get_cpu_idle_ticks(uint32_t cpu);
uint32_t sched_get_cpu_rq_size(uint32_t cpu);
int sched_get_thread_info(int tid, sched_thread_info_t* out);
int sched_max_threads(void);
void sched_dump(void);

void sched_demo_start(void);

#endif
