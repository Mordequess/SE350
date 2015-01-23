#ifndef MEMORY_H
#define MEMORY_H

#include "pcb.h"

#define RAM_END_ADDR 0x10008000

#define HEAP_START_ADDR 0x10007C00
#define HEAP_END_ADDR (0x10000000 + 4 * sizeof(pcb *))


extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;

typedef struct heap_blk {
	unsigned int start_Addr; //start address of this free space
	unsigned int length;
	unsigned int next_Addr;			 //points to next chunk of free space (used memory will be skipped over)
} heap_blk;

void memory_init(void);
void *request_memory_block(void);
int release_memory_block(void *memory_block);

#endif
