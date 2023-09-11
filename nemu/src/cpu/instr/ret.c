#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/

make_instr_func(ret) {
    cpu.eip = pop();
    return 0;
}

#define instr_execute_1op() do {\
        operand_read(psrc); \
        cpu.eip = pop(); \
        cpu.esp += vsrc; \
        len = 0; \
    } while (0)

make_instr_impl_1op(ret, i, w)
