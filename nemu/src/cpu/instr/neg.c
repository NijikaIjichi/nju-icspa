#include "cpu/instr.h"
/*
Put the implementations of `neg' instructions here.
*/

static void instr_execute_1op() {
    operand_read(psrc);
    vsrc = alu_sub(vsrc, 0, ssrc);
    operand_write(psrc);
}

make_instr_impl_1op(neg, rm, b)
make_instr_impl_1op(neg, rm, v)
