#include "cpu/instr.h"
/*
Put the implementations of `not' instructions here.
*/

static void instr_execute_1op() {
    operand_read(psrc);
    vsrc = alu_xor(vsrc, 0xffffffff, ssrc);
    operand_write(psrc);
}

make_instr_impl_1op(not, rm, b)
make_instr_impl_1op(not, rm, v)
