#include "common.h"
#include "memory.h"
#include "string.h"

#include <elf.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#define ELF_FILE_MAX_SIZE (1 << 20)
#endif

#define STACK_SIZE (1 << 20)

void ide_read(uint8_t *, uint32_t, uint32_t);
void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader()
{
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;

#ifdef HAS_DEVICE_IDE
    uint32_t elf_sz = in_long(0x1F8);
    if (elf_sz > ELF_FILE_MAX_SIZE) {
        elf_sz = ELF_FILE_MAX_SIZE;
    }
    uint8_t buf[elf_sz];
	ide_read(buf, ELF_OFFSET_IN_DISK, elf_sz);
	elf = (void *)buf;
	Log("ELF loading from hard disk.");
#else
	elf = (void *)0x0;
	Log("ELF loading from ram disk.");
#endif
    assert(*(uint32_t*)(elf->e_ident) == 0x464c457f);
    assert(elf->e_machine == EM_386);
	/* Load each program segment */
	ph = (void *)elf + elf->e_phoff;
	eph = ph + elf->e_phnum;
#ifdef HAS_DEVICE_IDE
    assert((uint32_t)eph <= (uint32_t)elf + elf_sz);
#endif
	for (; ph < eph; ph++)
	{
		if (ph->p_type == PT_LOAD)
		{
            uint32_t va = ph->p_vaddr;
#ifdef IA32_PAGE
			/* Record the program break for future use */
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if (brk < new_brk)
			{
				brk = new_brk;
			}
            va = (uint32_t)
                 (pa_to_va((mm_malloc(va, ph->p_memsz) & 0xfffff000) | (va & 0xfff)));
#endif
            memcpy((void*)va, (void*)((uint32_t)elf + ph->p_offset), 
                    ph->p_filesz);
            memset((void*)(va + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
		}
	}

	volatile uint32_t entry = elf->e_entry;
#ifdef IA32_PAGE
	mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
#ifdef HAS_DEVICE_VGA
	create_video_mapping();
#endif
	write_cr3(get_ucr3());
#endif
	return entry;
}
