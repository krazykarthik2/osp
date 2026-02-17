#include "proc.h"
#include "sched.h"
#include "terminal.h"

#define PROC_MAX 16
#define PROC_NAME_LEN 24

typedef struct {
    int used;
    int pid;
    int tid;
    uint32_t total_ticks;
    uint32_t remaining_ticks;
    char name[PROC_NAME_LEN];
} proc_t;

static proc_t procs[PROC_MAX];
static int next_pid = 100;

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

static void term_write_i32(int value) {
    if (value < 0) {
        terminal_putc('-');
        term_write_u32((uint32_t)(-value));
        return;
    }
    term_write_u32((uint32_t)value);
}

static void copy_name(char* dst, const char* src) {
    int i = 0;
    while (src[i] && i < PROC_NAME_LEN - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static const char* thread_state_name(uint8_t state) {
    if (state == SCHED_THREAD_READY) return "R";
    if (state == SCHED_THREAD_RUNNING) return "RUN";
    if (state == SCHED_THREAD_DONE) return "Z";
    return "U";
}

static int proc_worker_step(void* ctx) {
    proc_t* proc = (proc_t*)ctx;
    if (!proc->used) return 1;
    if (proc->remaining_ticks == 0) return 1;

    proc->remaining_ticks--;
    return proc->remaining_ticks == 0;
}

static int find_proc_slot_by_pid(int pid) {
    for (int i = 0; i < PROC_MAX; i++) {
        if (procs[i].used && procs[i].pid == pid) return i;
    }
    return -1;
}

void proc_init(void) {
    for (int i = 0; i < PROC_MAX; i++) {
        procs[i].used = 0;
        procs[i].pid = 0;
        procs[i].tid = -1;
        procs[i].total_ticks = 0;
        procs[i].remaining_ticks = 0;
        procs[i].name[0] = 0;
    }
    next_pid = 100;
}

int proc_spawn_bg(const char* name, uint32_t work_ticks) {
    if (!name || !name[0]) return -1;
    if (work_ticks == 0) work_ticks = 1;

    int slot = -1;
    for (int i = 0; i < PROC_MAX; i++) {
        if (!procs[i].used) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return -1;

    procs[slot].used = 1;
    procs[slot].pid = next_pid++;
    procs[slot].total_ticks = work_ticks;
    procs[slot].remaining_ticks = work_ticks;
    copy_name(procs[slot].name, name);

    int tid = sched_thread_create(procs[slot].name, 0, proc_worker_step, &procs[slot]);
    if (tid < 0) {
        procs[slot].used = 0;
        return -1;
    }

    procs[slot].tid = tid;
    return procs[slot].pid;
}

int proc_kill(int pid) {
    int slot = find_proc_slot_by_pid(pid);
    if (slot < 0) return -1;

    if (procs[slot].tid >= 0) {
        sched_thread_kill(procs[slot].tid);
    }
    procs[slot].remaining_ticks = 0;
    return 0;
}

void proc_ps(void) {
    terminal_writeln("PID   TID  STATE TICKS NAME");
    for (int i = 0; i < PROC_MAX; i++) {
        if (!procs[i].used) continue;

        sched_thread_info_t info;
        if (procs[i].tid < 0 || sched_get_thread_info(procs[i].tid, &info) != 0) continue;

        terminal_write(" ");
        term_write_i32(procs[i].pid);
        terminal_write("   ");
        term_write_i32(procs[i].tid);
        terminal_write("   ");
        terminal_write(thread_state_name(info.state));
        terminal_write("    ");
        term_write_u32(info.run_count);
        terminal_write("    ");
        terminal_writeln(procs[i].name);
    }
}

void proc_top(void) {
    uint32_t ticks = sched_get_tick_count();
    uint32_t cpus = sched_get_cpu_count();

    terminal_write("top - ticks: ");
    term_write_u32(ticks);
    terminal_write(" cpus: ");
    term_write_u32(cpus);
    terminal_putc('\n');

    terminal_writeln("CPU   SWITCH  IDLE");
    for (uint32_t cpu = 0; cpu < cpus; cpu++) {
        terminal_write(" ");
        term_write_u32(cpu);
        terminal_write("     ");
        term_write_u32(sched_get_cpu_switches(cpu));
        terminal_write("     ");
        term_write_u32(sched_get_cpu_idle_ticks(cpu));
        terminal_putc('\n');
    }

    terminal_writeln("PROC  STATE  CPU_TICKS  NAME");
    for (int i = 0; i < PROC_MAX; i++) {
        if (!procs[i].used) continue;

        sched_thread_info_t info;
        if (procs[i].tid < 0 || sched_get_thread_info(procs[i].tid, &info) != 0) continue;

        if (info.state == SCHED_THREAD_DONE && procs[i].remaining_ticks == 0) {
            continue;
        }

        terminal_write(" ");
        term_write_i32(procs[i].pid);
        terminal_write("    ");
        terminal_write(thread_state_name(info.state));
        terminal_write("     ");
        term_write_u32(info.run_count);
        terminal_write("       ");
        terminal_writeln(procs[i].name);
    }
}
