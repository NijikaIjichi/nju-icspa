#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `in' instructions here.
*/

static void instr_execute_2op() {
    operand_read(psrc);
    vdst = pio_read((uint16_t)vsrc, sdst / 8);
    operand_write(pdst);
}

make_instr_impl_2op(in, i, a, b)
make_instr_impl_2op(in, i, a, bv)
make_instr_impl_2op(in, wd, a, b)
make_instr_impl_2op(in, wd, a, v)
