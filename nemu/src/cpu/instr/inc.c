#include "cpu/instr.h"
/*
Put the implementations of `inc' instructions here.
*/

static void instr_execute_1op() {
    operand_read(psrc);
    vsrc = alu_add(1, vsrc, ssrc);
    operand_write(psrc);
}

make_instr_impl_1op(inc, rm, b)
make_instr_impl_1op(inc, rm, v)
make_instr_impl_1op(inc, r, v)
