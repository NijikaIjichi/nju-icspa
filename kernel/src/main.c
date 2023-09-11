#include "common.h"
#include "memory.h"
#include "proc.h"

void init_page();
void init_serial();
void init_ide();
void init_i8259();
void init_segment();
void init_idt();
void init_mm();

void create_video_mapping(PDE *pgdir);
void video_mapping_write_test();
void video_mapping_read_test();
void video_mapping_clear();

void init_cond();

extern size_t heap_start;
extern char end;

/* Initialization phase 1
 * The assembly code in start.S will finally jump here.
 */
void init()
{
#ifdef IA32_PAGE
	/* We must set up kernel virtual memory first because our kernel thinks it 
	 * is located at 0xc0030000, which is set by the linking options in Makefile.
	 * Before setting up correct paging, no global variable can be used. */
	init_page();

	/* After paging is enabled, transform %esp to virtual address. */
	asm volatile("addl %0, %%esp"
				 :
				 : "i"(KOFFSET));
#endif

/* Jump to init_cond() to continue initialization. */
// need to plus the offset 0xc0000000 if using gcc-6, strange
#ifdef IA32_PAGE
	asm volatile("jmp *%0"
				 :
				 : "r"(init_cond + 0xc0000000));
#else
	asm volatile("jmp *%0"
				 :
				 : "r"(init_cond));
#endif

	/* Should never reach here. */
	nemu_assert(0);
}

/* Initialization phase 2 */
void init_cond()
{
#ifdef IA32_INTR
	/* Reset the GDT, since the old GDT in start.S cannot be used in the future. */
	init_segment();

	/* Set the IDT by setting up interrupt and exception handlers.
	 * Note that system call is the only exception implemented in NEMU.
	 */
	init_idt();

	/* Enable interrupts. */
	// sti();
#endif

#ifdef HAS_DEVICE_IDE
	/* Initialize the IDE driver. */
	init_ide();
#endif

#ifdef IA32_PAGE
	/* Initialize the memory manager. */
	//init_mm();
#endif
    heap_start = PAGE_UP(&end);
    Log("Free physics mem start from 0x%08x.", heap_start);
	/* Output a welcome message.
	 * Note that the output is actually performed only when
	 * the serial port is available in NEMU.
	 */
	Log("Hello, NEMU world!");

#ifdef HAS_DEVICE_VGA
    create_video_mapping(get_kpdir());
	/* Write some test data to the video memory. */
	video_mapping_write_test();
#endif
	/* Load the program. */
	// uint32_t eip = loader();
    init_proc();
#ifdef HAS_DEVICE_VGA
	/* Read data in the video memory to check whether 
	 * the test data is written sucessfully.
	 */
	video_mapping_read_test();

	/* Clear the test data we just written in the video memory. */
	video_mapping_clear();
#endif
    Log("Init finish.");

    yield();

    panic("Should not reach here.");
}
