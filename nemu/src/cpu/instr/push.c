#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/

static void instr_execute_1op() {
    operand_read(psrc);
    push(vsrc);
}

make_instr_impl_1op(push, rm, v)
make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, si, b)
make_instr_impl_1op(push, si, v)

make_instr_func(pusha) {
    uint32_t esp = cpu.esp;
    push(cpu.eax);
    push(cpu.ecx);
    push(cpu.edx);
    push(cpu.ebx);
    push(esp);
    push(cpu.ebp);
    push(cpu.esi);
    push(cpu.edi);
    return 1;
}
