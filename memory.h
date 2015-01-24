#ifndef MEMORY_H
#define MEMORY_H

#include "k_rtx.h"

#define RAM_END_ADDR 0x10008000

extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
extern pcb **gp_pcbs;

typedef struct mem_blk {
	unsigned int *next_blk;
} mem_blk;

typedef struct heap_blk {
	unsigned int *start;
	unsigned int length;
} heap_blk;


void memory_init(void);
U32 *alloc_stack(U32 size_b);
void *k_request_memory_block(void);
int k_release_memory_block(void *);

#endif
