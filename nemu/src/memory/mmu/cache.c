#include "memory/mmu/cache.h"
#include "memory/memory.h"
#include <stdlib.h>

#define BLOCK_SIZE 64
#define ASSOCIATIVE 8
#define SET_NUM 128

#define OFFSET_MASK 0x3f
#define INDEX_MASK 0x1fc0
#define TAG_MASK 0xffffe000

typedef union {
    uint32_t val;
    struct {
        uint32_t offset: 6;
        uint32_t index: 7;
        uint32_t tag: 19;
    };
} paddr_u;

typedef struct {
    uint32_t valid: 1;
    uint32_t tag: 19;
    uint8_t data[BLOCK_SIZE];
} cache_block;

typedef cache_block cache_set[ASSOCIATIVE];

static cache_set cache[SET_NUM];
static int dirty = 0;

static inline cache_block *find_cache(paddr_u addr, bool alloc) {
    addr.offset = 0;
    cache_block *set = cache[addr.index], *empty = NULL;
    for (int i = 0; i < ASSOCIATIVE; ++i) {
        if (!set[i].valid) {
            empty = &set[i];
        } else if (set[i].tag == addr.tag) {
            return &set[i];
        }
    }
    if (!alloc) {
        return NULL;
    }
    if (!empty) {
        empty = &set[rand() % ASSOCIATIVE];
    }
    empty->valid = 1;
    empty->tag = addr.tag;
    memcpy(empty->data, hwa_to_va(addr.val), BLOCK_SIZE);
    return empty;
}

// init the cache
void init_cache()
{
	// implement me in PA 3-1
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	// implement me in PA 3-1
    paddr_u addr = {.val = paddr};
    if (addr.offset + len > BLOCK_SIZE) {
        for (int i = 0; i < len; ++i) {
            cache_write(paddr + i, 1, (uint32_t)(((uint8_t*)&data)[i]));
        }
        return;
    }
    cache_block *block = find_cache(addr, 0);
    if (block) {
        memcpy(&(block->data[addr.offset]), &data, len);
    }
    hw_mem_write(paddr, len, data);
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
    dirty = 1;
    uint32_t r = 0;
	paddr_u addr = {.val = paddr};
    if (addr.offset + len > BLOCK_SIZE) {
        for (int i = 0; i < len; ++i) {
            ((uint8_t*)&r)[i] = (uint8_t)cache_read(paddr + i, 1);
        }
        return r;
    }
    cache_block *block = find_cache(addr, 1);
    memcpy(&r, &(block->data[addr.offset]), len);
	return r;
}

void cache_clean() {
    if (dirty) {
        memset(cache, 0, sizeof(cache));
        dirty = 0;
    }
}

