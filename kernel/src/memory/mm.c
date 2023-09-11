#include "common.h"
#include "memory.h"
#include "proc.h"
#include <string.h>

PDE *get_kpdir();

size_t heap_start;

/* The brk() system call handler. */
void mm_brk(uint32_t new_brk)
{
    if (current->max_brk == 0) {
        current->max_brk = new_brk;
    } else if (new_brk > current->max_brk) {
		// mm_malloc(brk, new_brk - brk);
        map(current->pgdir, new_space(new_brk - current->max_brk), 
            current->max_brk, new_brk - current->max_brk);
        current->max_brk = PAGE_UP(new_brk);
	}
}

PDE *protect() {
    PDE *pgdir = new_space(PAGE_SIZE);
    PDE *kpdir = get_kpdir();
    memset(pgdir, 0, NR_PDE * sizeof(PDE));
    memcpy(&pgdir[KOFFSET / PT_SIZE], &kpdir[KOFFSET / PT_SIZE],
		   (PHY_MEM / PT_SIZE) * sizeof(PDE));
#ifdef HAS_DEVICE_VGA
	create_video_mapping(pgdir);
#endif
    return pgdir;
}

void *new_space(size_t len) {
    void *old_start = (void*)heap_start;
    heap_start += PAGE_UP(len);
    return old_start;
}

void map(PDE *page_dic, void *kva, size_t uva, size_t len) { 
    for (size_t i = PAGE_DOWN(kva); i < PAGE_UP(kva + len); 
            i += PAGE_SIZE, uva += PAGE_SIZE) {
        if (!page_dic[ADDR2DIR(uva)].present) {
            page_dic[ADDR2DIR(uva)].val = make_pde(va_to_pa(new_space(PAGE_SIZE)));
        }
        PTE *pgtbl = pa_to_va(page_dic[ADDR2DIR(uva)].page_frame << 12);
        pgtbl[ADDR2TBL(uva)].val = make_pte(va_to_pa(i));
    }
}
