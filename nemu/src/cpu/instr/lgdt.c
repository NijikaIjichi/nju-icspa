#include "cpu/instr.h"
/*
Put the implementations of `lgdt' instructions here.
*/

static void instr_execute_2op() {
    operand_read(psrc);
    operand_read(pdst);
    cpu.gdtr.limit = vsrc;
    cpu.gdtr.base = vdst;
}

make_instr_impl_2op(lgdt, wm, m, v)
