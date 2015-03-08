#ifndef K_MEMORY_H
#define K_MEMORY_H

#include "k_rtx.h"
#include "k_process.h"

extern pcb* g_ready_queue;
extern pcb* g_blocked_on_memory_queue;
extern pcb* g_blocked_on_receive_queue;
extern pcb **gp_pcbs;
extern U32 Image$$RW_IRAM1$$ZI$$Limit;

#define RAM_END_ADDR 0x10008000
#define BLOCK_SIZE 0x80

typedef struct heap_blk {
	U32 length;
	U32 next_Addr;
} heap_blk;

//heap allocate enough space for X (currently 100) memory blocks + one heap header
#define HEAP_START_ADDR (0x10001000 - sizeof(heap_blk *)) //arbitrarily chosen to leave enough space on either side
#define HEAP_END_ADDR 0x10001160 //(HEAP_START_ADDR + BLOCK_SIZE * 2 + sizeof(heap_blk *))

void memory_init(void);
U32 *alloc_stack(U32 size_b);
void *k_request_memory_block(void);
int k_release_memory_block(void *);

#endif
