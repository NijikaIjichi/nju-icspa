#include "cpu/intr.h"
#include "cpu/instr.h"

/*
Put the implementations of `int' instructions here.

Special note for `int': please use the instruction name `int_' instead of `int'.
*/

#define instr_execute_1op() do { \
    operand_read(psrc); \
    raise_sw_intr((uint8_t)vsrc); \
    len = 0; } while (0)

make_instr_impl_1op(int_, i, b)
