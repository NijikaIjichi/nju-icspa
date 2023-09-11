#include "cpu/instr.h"
/*
Put the implementations of `lea' instructions here.
*/

static void instr_execute_2op() {
    vdst = asrc;
    operand_write(pdst);
}

make_instr_impl_2op(lea, rm, r, v)
