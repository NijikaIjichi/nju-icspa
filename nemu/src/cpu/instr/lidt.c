#include "cpu/instr.h"
/*
Put the implementations of `lidt' instructions here.
*/

static void instr_execute_2op() {
    operand_read(psrc);
    operand_read(pdst);
    cpu.idtr.limit = vsrc;
    cpu.idtr.base = vdst;
}

make_instr_impl_2op(lidt, wm, m, v)
