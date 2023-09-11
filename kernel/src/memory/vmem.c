#include "common.h"
#include "memory.h"
#include "x86.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)
#define NR_PT ((SCR_SIZE + PT_SIZE - 1) / PT_SIZE) // number of page tables to cover the vmem

static PTE pgtbls[NR_PT][NR_PTE] align_to_page;

PDE *get_updir();

void create_video_mapping()
{

	/* create an identical mapping from virtual memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memeory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */
    uint32_t addr = VMEM_ADDR;
    PT pgtbl = pgtbls, pg;
	for (; addr < VMEM_ADDR + SCR_SIZE; ++pgtbl) {
        if (get_updir()[addr >> 22].present) {
            pg = pa_to_va((PT)(get_updir()[addr >> 22].page_frame << 12));
        } else {
            pg = pgtbl;
            get_updir()[addr >> 22].val = make_pde(va_to_pa(pg));
        }
        for (; addr < VMEM_ADDR + SCR_SIZE && ((addr >> 12) & 0x3ff) <= 0x3ff; 
               addr += PAGE_SIZE) {
            (*pg)[(addr >> 12) & 0x3ff].val = make_pte(addr);
        }
    }
}

void video_mapping_write_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		buf[i] = i;
	}
}

void video_mapping_read_test()
{
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for (i = 0; i < SCR_SIZE / 4; i++)
	{
		assert(buf[i] == i);
	}
}

void video_mapping_clear()
{
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}
