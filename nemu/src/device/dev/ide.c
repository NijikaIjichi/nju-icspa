#include "nemu.h"
#include "device/ide.h"
#include "device/i8259_pic.h"
#define MY_IDE

#ifndef MY_IDE
static uint32_t sector;
static uint32_t byte_cnt;
static bool ide_write;
static uint32_t disk_idx;
#else
#include "memory/memory.h"
#define BLK_SIZE 512

enum {
    ADDR = IDE_PORT_BASE,
    BLKNO = IDE_PORT_BASE + 4,
    SIZE = IDE_PORT_BASE + 8,
    RW = IDE_PORT_BASE + 12,    // 0: read, 1: write
    STATUS = IDE_PORT_BASE + 13 // 0: free, 1: work
};
#endif
static FILE *disk_fp;
static uint32_t disk_sz;

// init the hard disk by loading the file into it
void init_ide(const char *file_to_load)
{
	disk_fp = fopen(file_to_load, "rb+");
	assert(disk_fp != 0);
    fseek(disk_fp, 0, SEEK_END);
    disk_sz = ftell(disk_fp);
    fseek(disk_fp, 0, SEEK_SET);
    write_io_port(SIZE, 4, disk_sz);
    write_io_port(STATUS, 1, 0);
#ifndef MY_IDE
    disk_idx = 0;
    write_io_port(IDE_PORT_BASE + 8, 4, disk_sz);
    write_io_port(IDE_PORT_BASE + 7, 1, 0x40);
#else
    //printf("disk size: 0x%08x.\n", disk_sz);
#endif
}

make_pio_handler(handler_ide)
{
#ifndef MY_IDE
	uint32_t buf;
	assert(byte_cnt <= 512);
	if (is_write)
	{
		if (port - IDE_PORT_BASE == 0 && len == 4)
		{
			/* write 4 bytes data to disk */
			assert(ide_write);
			buf = read_io_port(IDE_PORT_BASE, 4);
			fwrite(&buf, 4, 1, disk_fp);

			byte_cnt += 4;
			if (byte_cnt == 512)
			{
				/* finish */
				write_io_port(IDE_PORT_BASE + 7, 1, 0x40);
			}
		}
		else if (port - IDE_PORT_BASE == 7)
		{
			if (read_io_port(IDE_PORT_BASE + 7, 1) == 0x20 || read_io_port(IDE_PORT_BASE + 7, 1) == 0x30)
			{
				/* command: read/write */
				sector = (read_io_port(IDE_PORT_BASE + 6, 1) & 0x1f) << 24 | read_io_port(IDE_PORT_BASE + 5, 1) << 16 | read_io_port(IDE_PORT_BASE + 4, 1) << 8 | read_io_port(IDE_PORT_BASE + 3, 1);
				disk_idx = sector << 9;
				fseek(disk_fp, disk_idx, SEEK_SET);

				byte_cnt = 0;

				if (read_io_port(IDE_PORT_BASE + 7, 1) == 0x20)
				{
					/* command: read from disk */
					ide_write = false;
					write_io_port(IDE_PORT_BASE + 7, 1, 0x40);
					i8259_raise_intr(IDE_IRQ);
				}
				else
				{
					/* command: write to disk */
					ide_write = true;
				}
			}
			else if (read_io_port(IDE_PORT_BASE + 7, 1) == 0xc8)
			{
				/* command: DMA read */

				/* Nothing to do here. The actual read operation is
				 * issued by write commands to the bus master register. */
			}
			else
			{
				/* not implemented command */
				assert(0);
			}
		}
	}
	else
	{
		if (port - IDE_PORT_BASE == 0 && len == 4)
		{
			/* read 4 bytes data from disk */
			assert(!ide_write);
			fread(&buf, 4, 1, disk_fp);
			write_io_port(IDE_PORT_BASE, 4, buf);
			byte_cnt += 4;
			if (byte_cnt == 512)
			{
				/* finish */
				write_io_port(IDE_PORT_BASE + 7, 1, 0x40);
			}
		}
	}
#else
    if (!is_write || port != STATUS || !read_io_port(STATUS, 1)) {return;}
    uint32_t pa_addr = read_io_port(ADDR, 4);
    void *hw_addr = hwa_to_va(pa_addr);
    uint32_t disk_idx = read_io_port(BLKNO, 4) * BLK_SIZE;
    uint32_t rw = read_io_port(RW, 1);
    if (disk_idx > disk_sz) {
        write_io_port(STATUS, 1, 0);
        return;
    }
    fseek(disk_fp, disk_idx, SEEK_SET);
    if (rw) {
        fwrite(hw_addr, BLK_SIZE, 1, disk_fp);
    } else {
#ifdef CACHE_ENABLED
        cache_clean();
#endif
        fread(hw_addr, BLK_SIZE, 1, disk_fp);
    }
    write_io_port(STATUS, 1, 0);
    /*printf("%c blkno. 0x%x with pa 0x%08x.\n", 
     *           rw ? 'W' : 'R', disk_idx / BLK_SIZE, pa_addr);
     */
#endif
}
