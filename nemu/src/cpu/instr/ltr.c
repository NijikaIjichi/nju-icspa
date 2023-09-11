#include "cpu/instr.h"
/*
Put the implementations of `ltr' instructions here.
*/

static void instr_execute_1op() {
    operand_read(psrc);
    cpu.tr = vsrc;
}

make_instr_impl_1op(ltr, rm, w)
