#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	return offset + cpu.segReg[sreg].base;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
    SegDesc desc;
    desc.val[0] = laddr_read(cpu.gdtr.base + 8 * cpu.segReg[sreg].index, 4);
    desc.val[1] = laddr_read(cpu.gdtr.base + 8 * cpu.segReg[sreg].index + 4, 4);
    cpu.segReg[sreg].base = desc.base_15_0 | (desc.base_23_16 << 16) | 
                            (desc.base_31_24 << 24);
    cpu.segReg[sreg].limit = desc.limit_15_0 | (desc.limit_19_16 << 16);
    assert(cpu.segReg[sreg].base == 0);
    assert(cpu.segReg[sreg].limit == 0xfffff);
    assert(desc.granularity == 1);
}
