#include "../include/sched.h"
#include "../include/console.h"

extern void switch_context(uint32_t* old_esp, uint32_t new_esp);

enum { TASK_UNUSED = 0, TASK_RUNNABLE = 1, TASK_RUNNING = 2 };

typedef struct task {
    const char* name;
    uint32_t esp;
    uint8_t state;
} task_t;

#define MAX_TASKS 8
#define STACK_SIZE 4096

static task_t tasks[MAX_TASKS];
static uint8_t stacks[MAX_TASKS][STACK_SIZE] __attribute__((aligned(16)));
static int current_task = -1;
static uint32_t bootstrap_esp = 0;

static uint32_t* push32(uint32_t* sp, uint32_t v) {
    sp--;
    *sp = v;
    return sp;
}

static void task_exit(void) {
    console_writeln("\n[task exited]");
    for (;;) sched_yield();
}

static void build_initial_stack(task_t* t, task_entry_t entry, int stack_index) {
    uint32_t stack_phys = (uint32_t)stacks[stack_index] + KERNEL_PHYS_BASE;
    uint32_t* sp = (uint32_t*)(stack_phys + STACK_SIZE);
    sp = (uint32_t*)((uint32_t)sp & ~0xFu);

    // When switch_context() finishes (popad; ret), it returns into entry.
    // If entry returns, it returns into task_exit.
    // Kernel is linked at 0 but loaded at KERNEL_PHYS_BASE, so adjust
    // function pointers to their actual runtime addresses.
    sp = push32(sp, (uint32_t)task_exit + KERNEL_PHYS_BASE);
    sp = push32(sp, (uint32_t)entry + KERNEL_PHYS_BASE);

    // popad frame: EDI,ESI,EBP,ESP(dummy),EBX,EDX,ECX,EAX
    sp = push32(sp, 0); // EAX
    sp = push32(sp, 0); // ECX
    sp = push32(sp, 0); // EDX
    sp = push32(sp, 0); // EBX
    sp = push32(sp, 0); // ESP dummy
    sp = push32(sp, 0); // EBP
    sp = push32(sp, 0); // ESI
    sp = push32(sp, 0); // EDI

    t->esp = (uint32_t)sp;
}

void sched_init(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].name = 0;
        tasks[i].esp = 0;
        tasks[i].state = TASK_UNUSED;
    }
    current_task = -1;
    bootstrap_esp = 0;
}

int task_spawn(const char* name, task_entry_t entry) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED) {
            tasks[i].name = name;
            tasks[i].state = TASK_RUNNABLE;
            build_initial_stack(&tasks[i], entry, i);
            return i;
        }
    }
    return -1;
}

static int pick_next(void) {
    int start = current_task;
    for (int i = 0; i < MAX_TASKS; i++) {
        int idx = (start + 1 + i) % MAX_TASKS;
        if (tasks[idx].state == TASK_RUNNABLE) return idx;
    }
    return -1;
}

void sched_yield(void) {
    int next = pick_next();
    if (next < 0) return;

    int prev = current_task;
    current_task = next;

    tasks[next].state = TASK_RUNNING;
    if (prev >= 0) tasks[prev].state = TASK_RUNNABLE;

    if (prev < 0) {
        switch_context(&bootstrap_esp, tasks[next].esp);
        return;
    }
    switch_context(&tasks[prev].esp, tasks[next].esp);
}

void sched_start(void) {
    __asm__ volatile("mov %%esp, %0" : "=r"(bootstrap_esp));
    sched_yield();
    // Only reached if a task yields back to bootstrap.
    for (;;) sched_yield();
}

void sched_dump_tasks(void) {
    console_writeln("Tasks:");
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED) continue;
        console_write(" - ");
        console_write(tasks[i].name ? tasks[i].name : "(unnamed)");
        console_write(" [");
        if (i == current_task) console_write("current ");
        if (tasks[i].state == TASK_RUNNABLE) console_write("runnable");
        else if (tasks[i].state == TASK_RUNNING) console_write("running");
        else console_write("?");
        console_writeln("]");
    }
}
