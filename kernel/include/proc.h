#ifndef __PROC_H__
#define __PROC_H__

#include "common.h"
#include "x86.h"

#define KSTACK_SIZE (1 << 20)
#define USTACK_SIZE (1 << 20)

enum procstate { UNUSED, RUNNABLE, RUNNING };

typedef union {
    uint8_t kstack[KSTACK_SIZE] align_to_page;
    struct {
        TrapFrame *ctx;
        PDE *pgdir;
        size_t max_brk;
        enum procstate state;
        uint32_t priority;
        uint32_t pid;
    };
} PCB;

extern PCB *current;

void init_proc();
TrapFrame *schedule(TrapFrame *prev, int force);

#endif
