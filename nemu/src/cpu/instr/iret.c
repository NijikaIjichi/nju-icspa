#include "cpu/instr.h"
/*
Put the implementations of `iret' instructions here.
*/

make_instr_func(iret) {
    cpu.eip = pop();
    cpu.cs.val = (uint16_t)pop();
    load_sreg(SREG_CS);
    cpu.eflags.val = pop();
    if (cpu.cs.rpl == 3) {
        uint32_t new_esp = pop();
        cpu.ss.val = pop();
        cpu.esp = new_esp;
        load_sreg(SREG_SS);
    }
    return 0;
}
