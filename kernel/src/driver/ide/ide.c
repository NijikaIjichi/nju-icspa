#include "common.h"

#define WRITEBACK_TIME 1 /* writeback cache for every 1 second */
#define HZ 100
#define MIN(a, b)  ((a) > (b) ? (b) : (a))

void cache_init(void);
void cache_writeback(void);
// uint8_t read_byte(uint32_t);
uint8_t *read_blk(uint32_t sector);
void write_byte(uint32_t, uint8_t);

void add_irq_handle(int, void (*)(void));

static void kmemcpy(uint8_t *dst, const uint8_t *src, uint32_t len) {
    uint32_t i;
    uint64_t *dst_64 = (uint64_t*)dst; 
    const uint64_t *src_64 = (const uint64_t*)src;
    for (i = 0; i < len / 8; ++i) {
        dst_64[i] = src_64[i];
    }
    for (i *= 8; i < len; ++i) {
        dst[i] = src[i];
    }
}

/* The loader acts as a monolithic kernel, therefore we do not need
 * to translate the address ``buf'' from the user process to a physical
 * one, which is necessary for a microkernel.
 */
void ide_read(uint8_t *buf, uint32_t offset, uint32_t len)
{
	/*uint32_t i;
	 *for (i = 0; i < len; i++)
	 *{
	 *	buf[i] = read_byte(offset + i);
	 *}
     */
    uint32_t read_end = offset + len;
    assert(read_end >= offset);
    for (; offset < read_end; offset = (offset / 512 + 1) * 512) {
        uint8_t *blk = read_blk(offset / 512);
        uint32_t copy_len = MIN(read_end - offset, 512 - offset % 512);
        kmemcpy(buf, &blk[offset % 512], copy_len);
        buf += copy_len;
    }
}

void ide_write(uint8_t *buf, uint32_t offset, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i++)
	{
		write_byte(offset + i, buf[i]);
	}
}

static void
ide_writeback(void)
{
	static uint32_t counter = 0;
	counter++;
	if (counter == WRITEBACK_TIME * HZ)
	{
		cache_writeback();
		counter = 0;
	}
}

static volatile int has_ide_intr;

static void
ide_intr(void)
{
	has_ide_intr = 1;
}

void clear_ide_intr(void)
{
	has_ide_intr = 0;
}

void wait_ide_intr(void)
{
	while (has_ide_intr == 0)
	{
		wait_intr();
	}

	clear_ide_intr();
}

void init_ide(void)
{
	cache_init();
	add_irq_handle(0, ide_writeback);
	add_irq_handle(14, ide_intr);
}
