#ifndef MEMORY_H
#define MEMORY_H
#define RAM_END_ADDR 0x10008000

#include "pcb.h"

extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
pcb **gp_pcbs;

typedef struct mem_blk {
	unsigned int *next_blk;
} mem_blk;

typedef struct heap_blk {
	unsigned int *start;
	unsigned int length;
	bool used;
} heap_blk;

void memory_init(void);

void *request_memory_block(void);

int release_memory_block(void *memory_block);

#endif
