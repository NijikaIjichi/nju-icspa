#include "common.h"
#include "memory.h"
#include "string.h"
#include "proc.h"
#include <elf.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)
#define MAX_ELF_SIZE (1 << 20)

int fs_open(const char *pathname, int flags);
size_t fs_read(int fd, void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
enum {SEEK_SET, SEEK_CUR, SEEK_END};

uint32_t loader_from_file(const char *elf_name, PCB *pcb) {
    int fd = fs_open(elf_name, 0);
    if (fd < 0) {
        return 0;
    }
	Elf32_Ehdr elf;
	Elf32_Phdr ph;
    fs_read(fd, &elf, sizeof(elf));
    assert(*(uint32_t*)(elf.e_ident) == 0x464c457f);
    assert(elf.e_machine == EM_386);
	for (size_t i = 0; i < elf.e_phnum; i++) {
        fs_lseek(fd, elf.e_phoff + i * sizeof(ph), SEEK_SET);
        fs_read(fd, &ph, sizeof(ph));
		if (ph.p_type == PT_LOAD) {
            uint32_t va = ph.p_vaddr;
#ifdef IA32_PAGE
            va = ((uint32_t)new_space(ph.p_memsz)) | ADDR2OFF(va);
            map(pcb->pgdir, (void*)va, ph.p_vaddr, ph.p_memsz);
#endif
            fs_lseek(fd, ph.p_offset, SEEK_SET);
            fs_read(fd, (void*)va, ph.p_filesz);
            memset((void*)(va + ph.p_filesz), 0, ph.p_memsz - ph.p_filesz);
		}
	}
    fs_close(fd);
	return elf.e_entry;
}
