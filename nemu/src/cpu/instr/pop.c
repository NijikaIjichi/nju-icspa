#include "cpu/instr.h"
/*
Put the implementations of `pop' instructions here.
*/

static void instr_execute_1op() {
    vsrc = pop();
    operand_write(psrc);
}

make_instr_impl_1op(pop, rm, v)
make_instr_impl_1op(pop, r, v)

make_instr_func(popa) {
    cpu.edi = pop();
    cpu.esi = pop();
    cpu.ebp = pop();
    pop();
    cpu.ebx = pop();
    cpu.edx = pop();
    cpu.ecx = pop();
    cpu.eax = pop();
    return 1;
}
