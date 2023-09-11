#include "common.h"
#include "memory.h"
#include "proc.h"

#define MAX_NR_PROC 4
static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
static uint32_t total_pid = 0;
PCB *current = NULL;

uint32_t loader_from_file(const char *elf_name, PCB *pcb);

TrapFrame *ucontext(void *kstack_top, uint32_t entry, void *pgdir) {
    TrapFrame *ctx = &((TrapFrame*)kstack_top)[-1];
    ctx->cs = SELECTOR_USER(1);
    ctx->ss3 = SELECTOR_USER(2);
    ctx->eflags = 0x202;
    ctx->eip = entry;
    ctx->ebp = KOFFSET;
    ctx->esp3 = KOFFSET;
    ctx->cr3 = va_to_pa(pgdir);
    return ctx;
}

TrapFrame *kcontext(void *kstack_top, uint32_t entry) {
    TrapFrame *ctx = &((TrapFrame*)kstack_top)[-1];
    ctx->cs = SELECTOR_KERNEL(1);
    ctx->eflags = 0x202;
    ctx->eip = entry;
    ctx->ebp = (uint32_t)kstack_top;
    ctx->cr3 = 0;
    return ctx;
}

void hello_fun() {
    int i = 0;
    while (1) {
        i++;
        Log("Hello from kernel thread (pid = %d) for the %dth time!", 
            current->pid, i);
        yield();
    }
}

void switch_boot_pcb() {
    current = &pcb_boot;
    pcb_boot.state = RUNNING;
    pcb_boot.priority = 0;
    set_kstack(&(pcb_boot.kstack[KSTACK_SIZE]));
}

PCB *alloc_proc() {
    for (size_t i = 0; i < MAX_NR_PROC; ++i) {
        if (pcb[i].state == UNUSED) {
            return &pcb[i];
        }
    }
    return NULL;
}

int context_uload(const char *elf_name, uint32_t priority) {
    PCB *pcb = alloc_proc();
    if (pcb == NULL) {
        return -1;
    }
    pcb->pgdir = protect();
    uint32_t entry = loader_from_file(elf_name, pcb);
    if (entry == 0) {
        return -1;
    }
    pcb->ctx = ucontext(&(pcb->kstack[KSTACK_SIZE]), entry, pcb->pgdir);
    map(pcb->pgdir, new_space(USTACK_SIZE), KOFFSET - USTACK_SIZE, USTACK_SIZE);
    pcb->state = RUNNABLE;
    pcb->priority = priority;
    ++total_pid;
    pcb->pid = total_pid;
    return 0;
}

int context_kload(uint32_t entry, uint32_t priority) {
    PCB *pcb = alloc_proc();
    if (pcb == NULL) {
        return -1;
    }
    pcb->pgdir = 0;
    pcb->ctx = kcontext(&(pcb->kstack[KSTACK_SIZE]), entry);
    pcb->state = RUNNABLE;
    pcb->priority = priority;
    ++total_pid;
    pcb->pid = total_pid;
    return 0;
}

void init_proc() {
    context_uload("main", 8);
    //context_uload("sub", 0);
    //context_kload((uint32_t)hello_fun, 0);
    switch_boot_pcb();
}

TrapFrame *schedule(TrapFrame *prev, int force) {
    static uint32_t i = 0;
    int start, end;
    current->ctx = prev;
    i++;
    if (force || current->state != RUNNING || (i >> current->priority) > 0) {
        i = 0;
        if (current == &pcb_boot) {
            start = end = MAX_NR_PROC;
        } else {
            start = end = current - pcb;
        }
        for (start = (start + 1) % MAX_NR_PROC; 
             start != end; start = (start + 1) % MAX_NR_PROC) {
            if (pcb[start].state == RUNNABLE) {
                if (current->state == RUNNING) {
                    current->state = RUNNABLE;
                }
                current = &pcb[start];
                current->state = RUNNING;
                break;
            }
        }
    }
    if (current->state == UNUSED) {
        HIT_GOOD_TRAP;
    } else if (current->state != RUNNING) {
        HIT_BAD_TRAP;
    }
    set_kstack(&(current->kstack[KSTACK_SIZE]));
    return current->ctx;
}
