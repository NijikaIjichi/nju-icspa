#include "cpu/instr.h"

static void instr_execute_1op() {
    operand_read(psrc);
    cpu.eip += vsrc;
}

make_instr_impl_1op(jmp, si, b)
make_instr_impl_1op(jmp, si, v)

#define instr_execute_1op() do {\
        operand_read(psrc); \
        cpu.eip = vsrc; \
        len = 0; \
    } while (0)

make_instr_impl_1op(jmp, rm, v)

#define instr_execute_2op() do {\
        operand_read(psrc); \
        operand_read(pdst); \
        cpu.cs.val = (uint16_t)vsrc; \
        load_sreg(SREG_CS); \
        cpu.eip = vdst; \
        len = 0; \
    } while (0)

make_instr_impl_2op(jmp, wi, i, v)
make_instr_impl_2op(jmp, wm, m, v)
