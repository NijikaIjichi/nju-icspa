#include "cpu/instr.h"
/*
Put the implementations of `iret' instructions here.
*/

make_instr_func(iret) {
    cpu.eip = pop();
    cpu.cs.val = (uint16_t)pop();
    load_sreg(SREG_CS);
    cpu.eflags.val = pop();
    return 0;
}
