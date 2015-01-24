#ifndef K_MEMORY_H
#define K_MEMORY_H

#include "k_rtx.h"

extern pcb **gp_pcbs;
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;

#define RAM_END_ADDR 0x10008000
#define HEAP_START_ADDR 0x10007C00
#define HEAP_END_ADDR (0x10000000 + 4 * sizeof(pcb *))

typedef struct heap_blk {
	unsigned int start_Addr; //start address of this free space
	unsigned int length;
	unsigned int next_Addr; //points to next chunk of free space (used memory will be skipped over)
} heap_blk;

void memory_init(void);
U32 *alloc_stack(U32 size_b);
void *k_request_memory_block(void);
int k_release_memory_block(void *);			 

#endif