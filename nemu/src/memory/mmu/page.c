#include "cpu/cpu.h"
#include "memory/memory.h"

typedef union {
    struct {
        uint32_t offset: 12;
        uint32_t page: 10;
        uint32_t dir: 10;
    };
    uint32_t val;
} laddr_u;

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
    laddr_u addr = {.val = laddr};
	uint32_t pgdir = cpu.cr3.dir << 12;
    uint32_t pgtbl = paddr_read(pgdir + addr.dir * 4, 4);
    assert(pgtbl & 1);
    pgtbl &= 0xfffff000;
    uint32_t paddr = paddr_read(pgtbl + addr.page * 4, 4);
    assert(paddr & 1);
    return (paddr & 0xfffff000) | addr.offset;
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
