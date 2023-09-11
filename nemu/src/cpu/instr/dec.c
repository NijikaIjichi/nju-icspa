#include "cpu/instr.h"
/*
Put the implementations of `dec' instructions here.
*/

static void instr_execute_1op() {
    operand_read(psrc);
    vsrc = alu_sub(1, vsrc, ssrc);
    operand_write(psrc);
}

make_instr_impl_1op(dec, rm, b)
make_instr_impl_1op(dec, rm, v)
make_instr_impl_1op(dec, r, v)
