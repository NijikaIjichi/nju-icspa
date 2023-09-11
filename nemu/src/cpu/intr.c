#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
    push(cpu.eflags.val);
    push(cpu.cs.val);
    push(cpu.eip);
	GateDesc desc;
    desc.val[0] = laddr_read(cpu.idtr.base + intr_no * 8, 4);
    desc.val[1] = laddr_read(cpu.idtr.base + intr_no * 8 + 4, 4);
    assert(desc.present);
    if (desc.type == 0xE) {
        cpu.eflags.IF = 0;
    }
    cpu.cs.val = desc.selector;
    cpu.eip = desc.offset_15_0 | (desc.offset_31_16 << 16);
    load_sreg(SREG_CS);
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
