#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/

#define instr_execute_1op() do {\
        operand_read(psrc); \
        push(cpu.eip + len); \
        cpu.eip += vsrc; \
    } while (0)

make_instr_impl_1op(call, si, v)

#undef instr_execute_1op
#define instr_execute_1op() do {\
        operand_read(psrc); \
        push(cpu.eip + len); \
        cpu.eip = vsrc; \
        len = 0; \
    } while (0)

make_instr_impl_1op(call, rm, v)
