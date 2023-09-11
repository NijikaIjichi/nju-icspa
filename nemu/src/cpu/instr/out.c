#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `out' instructions here.
*/

static void instr_execute_2op() {
    operand_read(psrc);
    operand_read(pdst);
    pio_write((uint16_t)vsrc, sdst / 8, vdst);
}

make_instr_impl_2op(out, i, a, b)
make_instr_impl_2op(out, i, a, bv)
make_instr_impl_2op(out, wd, a, b)
make_instr_impl_2op(out, wd, a, v)
