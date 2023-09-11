#include "common.h"
#include "x86.h"
#include "memory.h"

//#define USE_DMA_READ
#define MY_IDE
#define IDE_PORT_BASE 0x1F0

#ifndef MY_IDE
void dma_prepare(void *);
void dma_issue_read(void);

void clear_ide_intr(void);
void wait_ide_intr(void);
#else
enum {
    ADDR = IDE_PORT_BASE,
    BLKNO = IDE_PORT_BASE + 4,
    SIZE = IDE_PORT_BASE + 8,
    RW = IDE_PORT_BASE + 12,    // 0: read, 1: write
    STATUS = IDE_PORT_BASE + 13 // 0: free, 1: work
};
#endif

static void waitdisk()
{
#ifndef MY_IDE
	while ((in_byte(IDE_PORT_BASE + 7) & (0x80 | 0x40)) != 0x40)
		;
#else
    while (in_byte(STATUS)) ;
#endif
}

#ifndef MY_IDE
static void
ide_prepare(uint32_t sector)
{
	waitdisk();

#ifdef USE_DMA_READ
	out_byte(IDE_PORT_BASE + 1, 1);
#else
	out_byte(IDE_PORT_BASE + 1, 0);
#endif

	out_byte(IDE_PORT_BASE + 2, 1);
	out_byte(IDE_PORT_BASE + 3, sector & 0xFF);
	out_byte(IDE_PORT_BASE + 4, (sector >> 8) & 0xFF);
	out_byte(IDE_PORT_BASE + 5, (sector >> 16) & 0xFF);
	out_byte(IDE_PORT_BASE + 6, 0xE0 | ((sector >> 24) & 0xFF));
}
#endif

#ifndef MY_IDE
static inline void
issue_read()
{
#ifdef USE_DMA_READ
	out_byte(IDE_PORT_BASE + 7, 0xc8);
	dma_issue_read();
#else
	out_byte(IDE_PORT_BASE + 7, 0x20);
#endif
}
#endif

#ifndef MY_IDE
static inline void
issue_write()
{
	out_byte(IDE_PORT_BASE + 7, 0x30);
}
#endif

void disk_do_read(void *buf, uint32_t sector)
{
#ifndef MY_IDE
#ifdef USE_DMA_READ
	dma_prepare(buf);
#endif

	clear_ide_intr();

	ide_prepare(sector);
	issue_read();

	wait_ide_intr();

#ifndef USE_DMA_READ
	int i;
	for (i = 0; i < 512 / sizeof(uint32_t); i++)
	{
		*(((uint32_t *)buf) + i) = in_long(IDE_PORT_BASE);
	}
#endif
#else
    out_long(ADDR, (uint32_t)va_to_pa(buf));
    out_long(BLKNO, sector);
    out_byte(RW, 0);
    out_byte(STATUS, 1);
    waitdisk();
#endif
}

void disk_do_write(void *buf, uint32_t sector)
{
#ifndef MY_IDE
	int i;

	ide_prepare(sector);
	issue_write();

	for (i = 0; i < 512 / sizeof(uint32_t); i++)
	{
		out_long(IDE_PORT_BASE, *(((uint32_t *)buf) + i));
	}
#else
    out_long(ADDR, (uint32_t)va_to_pa(buf));
    out_long(BLKNO, sector);
    out_byte(RW, 1);
    out_byte(STATUS, 1);
    waitdisk();
#endif
}

size_t disk_sz() {
#ifndef MY_IDE
    return in_long(IDE_PORT_BASE + 8);
#else
    return in_long(SIZE);
#endif
}
