#include "sched.h"
#include "terminal.h"

#define SCHED_MAX_CPUS 8
#define SCHED_MAX_THREADS 32
#define DEMO_THREADS 4

enum {
    KTHREAD_UNUSED = 0,
    KTHREAD_READY = 1,
    KTHREAD_RUNNING = 2,
    KTHREAD_DONE = 3,
};

typedef struct {
    const char* name;
    kthread_step_t step;
    void* ctx;
    uint32_t affinity_mask;
    uint8_t state;
    uint32_t run_count;
} kthread_t;

typedef struct {
    int current_tid;
    uint32_t switches;
    uint32_t idle_ticks;
    int run_queue[SCHED_MAX_THREADS];
    int run_queue_size;
    int run_queue_cursor;
} cpu_state_t;

typedef struct {
    const char* label;
    uint32_t remaining;
    uint32_t period;
    uint32_t counter;
} demo_ctx_t;

static kthread_t threads[SCHED_MAX_THREADS];
static cpu_state_t cpus[SCHED_MAX_CPUS];
static uint32_t sched_cpu_count = 1;
static uint32_t sched_tick = 0;

static demo_ctx_t demo_ctx[DEMO_THREADS] = {
    { "worker-A", 30, 5, 0 },
    { "worker-B", 25, 4, 0 },
    { "worker-C", 20, 3, 0 },
    { "worker-D", 15, 2, 0 },
};

static void term_write_u32(uint32_t value) {
    char buf[16];
    int i = 0;
    if (value == 0) {
        terminal_putc('0');
        return;
    }
    while (value > 0 && i < (int)sizeof(buf)) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (i-- > 0) terminal_putc(buf[i]);
}

static void cpu_add_to_queue(uint32_t cpu, int tid) {
    if (cpu >= sched_cpu_count) return;
    if (cpus[cpu].run_queue_size >= SCHED_MAX_THREADS) return;
    cpus[cpu].run_queue[cpus[cpu].run_queue_size++] = tid;
}

static void cpu_remove_from_queue(uint32_t cpu, int tid) {
    if (cpu >= sched_cpu_count) return;
    for (int i = 0; i < cpus[cpu].run_queue_size; i++) {
        if (cpus[cpu].run_queue[i] == tid) {
            for (int j = i; j < cpus[cpu].run_queue_size - 1; j++) {
                cpus[cpu].run_queue[j] = cpus[cpu].run_queue[j + 1];
            }
            cpus[cpu].run_queue_size--;
            return;
        }
    }
}

static int pick_next_thread(uint32_t cpu) {
    if (cpus[cpu].run_queue_size == 0) return -1;
    
    // Pick from the per-core run queue using round-robin cursor
    for (int i = 0; i < cpus[cpu].run_queue_size; i++) {
        cpus[cpu].run_queue_cursor = (cpus[cpu].run_queue_cursor + 1) % cpus[cpu].run_queue_size;
        int tid = cpus[cpu].run_queue[cpus[cpu].run_queue_cursor];
        if (threads[tid].state == KTHREAD_READY) {
            return tid;
        }
    }
    return -1;
}

static int demo_worker_step(void* ctx) {
    demo_ctx_t* worker = (demo_ctx_t*)ctx;
    if (worker->remaining == 0) return 1;

    worker->counter++;
    worker->remaining--;

    if ((worker->counter % worker->period) == 0 || worker->remaining == 0) {
        terminal_write("[sched] ");
        terminal_write(worker->label);
        terminal_write(" tick=");
        term_write_u32(sched_tick);
        terminal_write(" remaining=");
        term_write_u32(worker->remaining);
        terminal_putc('\n');
    }

    return worker->remaining == 0;
}

void sched_init(uint32_t cpu_count) {
    if (cpu_count == 0) cpu_count = 1;
    if (cpu_count > SCHED_MAX_CPUS) cpu_count = SCHED_MAX_CPUS;

    sched_cpu_count = cpu_count;
    sched_tick = 0;

    for (int i = 0; i < SCHED_MAX_THREADS; i++) {
        threads[i].name = 0;
        threads[i].step = 0;
        threads[i].ctx = 0;
        threads[i].affinity_mask = 0;
        threads[i].state = KTHREAD_UNUSED;
        threads[i].run_count = 0;
    }

    for (uint32_t i = 0; i < sched_cpu_count; i++) {
        cpus[i].current_tid = -1;
        cpus[i].switches = 0;
        cpus[i].idle_ticks = 0;
        cpus[i].run_queue_size = 0;
        cpus[i].run_queue_cursor = -1;
    }

    terminal_write("Scheduler: initialized CPUs=");
    term_write_u32(sched_cpu_count);
    terminal_putc('\n');
}

int sched_thread_create(const char* name, uint32_t affinity_mask, kthread_step_t step, void* ctx) {
    if (!step) return -1;
    int tid = -1;
    for (int itid = 0; itid < SCHED_MAX_THREADS; itid++) {
        if (threads[itid].state == KTHREAD_UNUSED || threads[itid].state == KTHREAD_DONE) {
            tid = itid;
            break;
        }
    }
    if (tid < 0) return -1;

    threads[tid].name = name;
    threads[tid].step = step;
    threads[tid].ctx = ctx;
    threads[tid].affinity_mask = affinity_mask;
    threads[tid].state = KTHREAD_READY;
    threads[tid].run_count = 0;

    // Load balancing: assign to the CPU with smallest run queue that satisfies affinity
    uint32_t best_cpu = 0xFFFFFFFF;
    int min_load = 1000000;

    for (uint32_t c = 0; c < sched_cpu_count; c++) {
        if (affinity_mask != 0 && !(affinity_mask & (1u << c))) continue;
        if (cpus[c].run_queue_size < min_load) {
            min_load = cpus[c].run_queue_size;
            best_cpu = c;
        }
    }

    if (best_cpu == 0xFFFFFFFF) {
        // Fallback to first core if affinity mask is impossible
        best_cpu = 0;
    }

    cpu_add_to_queue(best_cpu, tid);
    return tid;
}

int sched_thread_kill(int tid) {
    if (tid < 0 || tid >= SCHED_MAX_THREADS) return -1;
    if (threads[tid].state == KTHREAD_UNUSED || threads[tid].state == KTHREAD_DONE) return -1;
    threads[tid].state = KTHREAD_DONE;
    
    // Remove from all CPU queues (clean cleanup)
    for (uint32_t c = 0; c < sched_cpu_count; c++) {
        cpu_remove_from_queue(c, tid);
    }
    return 0;
}

void sched_run_ticks(uint32_t ticks) {
    for (uint32_t t = 0; t < ticks; t++) {
        sched_tick++;

        for (uint32_t cpu = 0; cpu < sched_cpu_count; cpu++) {
            int next = pick_next_thread(cpu);
            if (next < 0) {
                cpus[cpu].idle_ticks++;
                continue;
            }

            cpus[cpu].current_tid = next;
            cpus[cpu].switches++;

            threads[next].state = KTHREAD_RUNNING;
            threads[next].run_count++;

            int done = threads[next].step(threads[next].ctx);
            if (done) {
                threads[next].state = KTHREAD_DONE;
                cpu_remove_from_queue(cpu, next);
            } else {
                threads[next].state = KTHREAD_READY;
            }
        }
    }
}


uint32_t sched_get_tick_count(void) {
    return sched_tick;
}

uint32_t sched_get_cpu_count(void) {
    return sched_cpu_count;
}

uint32_t sched_get_cpu_switches(uint32_t cpu) {
    if (cpu >= sched_cpu_count) return 0;
    return cpus[cpu].switches;
}

uint32_t sched_get_cpu_idle_ticks(uint32_t cpu) {
    if (cpu >= sched_cpu_count) return 0;
    return cpus[cpu].idle_ticks;
}

uint32_t sched_get_cpu_rq_size(uint32_t cpu) {
    if (cpu >= sched_cpu_count) return 0;
    return cpus[cpu].run_queue_size;
}

int sched_get_thread_info(int tid, sched_thread_info_t* out) {
    if (!out) return -1;
    if (tid < 0 || tid >= SCHED_MAX_THREADS) return -1;
    out->tid = tid;
    out->name = threads[tid].name;
    out->affinity_mask = threads[tid].affinity_mask;
    out->state = threads[tid].state;
    out->run_count = threads[tid].run_count;
    return 0;
}

int sched_max_threads(void) {
    return SCHED_MAX_THREADS;
}

void sched_dump(void) {
    terminal_writeln("Scheduler dump:");
    for (uint32_t cpu = 0; cpu < sched_cpu_count; cpu++) {
        terminal_write("  CPU");
        term_write_u32(cpu);
        terminal_write(" switches=");
        term_write_u32(cpus[cpu].switches);
        terminal_write(" idle=");
        term_write_u32(cpus[cpu].idle_ticks);
        terminal_putc('\n');
    }

    for (int tid = 0; tid < SCHED_MAX_THREADS; tid++) {
        if (threads[tid].state == KTHREAD_UNUSED) continue;
        terminal_write("  T");
        term_write_u32((uint32_t)tid);
        terminal_write(" ");
        terminal_write(threads[tid].name ? threads[tid].name : "(unnamed)");
        terminal_write(" runs=");
        term_write_u32(threads[tid].run_count);
        terminal_write(" state=");

        if (threads[tid].state == KTHREAD_READY) terminal_write("ready");
        else if (threads[tid].state == KTHREAD_RUNNING) terminal_write("running");
        else if (threads[tid].state == KTHREAD_DONE) terminal_write("done");
        else terminal_write("unused");

        terminal_putc('\n');
    }
}

void sched_demo_start(void) {
    sched_thread_create("worker-A", 0x1, demo_worker_step, &demo_ctx[0]);
    sched_thread_create("worker-B", 0x2, demo_worker_step, &demo_ctx[1]);
    sched_thread_create("worker-C", 0x3, demo_worker_step, &demo_ctx[2]);
    sched_thread_create("worker-D", 0x0, demo_worker_step, &demo_ctx[3]);

    terminal_writeln("Scheduler: demo threads created");
}
