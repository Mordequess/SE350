#include "k_memory.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

/*
0x10000000+---------------------------+ Low Address
          |       RTX  Image          |
          |---------------------------|
          |Image$$RW_IRAM1$$ZI$$Limit |
          |---------------------------|
          |        Padding            |
          |---------------------------|<--- gp_pcbs
          |        PCB pointers       |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB 2              |
          |---------------------------|
          |		space for more PCBs   |
          |---------------------------|<--- HEAP_START_ADDR (0x10003FFC)
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|<--- HEAP_END_ADDR (0x10007200)
          |	   space for more stacks  |
          |---------------------------|<--- gp_stack
          |    Proc 2 STACK           |
          |...........................|          
          |    Proc 1 STACK           |
          |                           |
0x10008000+---------------------------+ High Address
*/

/* ----- Global Variables ----- */
U32 *gp_stack;
	/* The last allocated stack low address. 8 bytes aligned */
	/* The first stack starts at the RAM high address */
	/* stack grows down. Fully decremental stack */

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

// the hardcoded stack size is STACK_SIZE = 0x100 or 256 U32s which means 256 x 4 bytes = 1024 bytes
U32 *alloc_stack(U32 size_b) 
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (U32 *)((unsigned char *)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}

void memory_init(void){
	
	//p_end is going to point to the end of the heap once all is allocated
	unsigned char *p_end = (unsigned char *)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;
	heap_blk* heap_Head;
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (pcb **)p_end;
	p_end += NUM_PROCESSES * sizeof(pcb *);
  	
	for (i = 0; i < NUM_PROCESSES; i++ ) {
		gp_pcbs[i] = (pcb *)p_end;
		p_end += sizeof(pcb); 
	}
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
	
	//allocate memory for heap

	//~ using p_end, we're taking out the reliance on the macro. (i'm on a dislike macros rave)
	heap_Head = (heap_blk*)HEAP_START_ADDR; //p_end;
	heap_Head->next_Addr = NULL;
	heap_Head->length = HEAP_END_ADDR - HEAP_START_ADDR - sizeof(heap_blk *); //length in heap_Head adjusts for header, others won't
	//~ this new calculation takes into account some space for the process stacks. (and the size of a heap block)

#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
	printf("stack total: 0x%x\n", ((U32)(NUM_PROCESSES*STACK_SIZE)));
	printf("heap start addr: 0x%x\n", ((U32)HEAP_START_ADDR));
	printf("heap end addr: 0x%x\n", ((U32)HEAP_END_ADDR));
	printf("heap length: 0x%x\n", (heap_Head->length));
#endif

}

void *k_request_memory_block(void) {
	//start at the top of the heap by making a pointer to HEAP_START_ADDR
	heap_blk* temp;
	heap_blk* freeNode = (heap_blk*)HEAP_START_ADDR;
	void* mem_blk;

	__disable_irq(); //atomic(on);

	//find the next free space large enough to fulfill the request
	while (freeNode->length < BLOCK_SIZE) {
		if (freeNode->next_Addr == 0){ //if this is last free node, the request can't be filled
			block_current_process(); //will release processor
			return NULL; 
		}
		else freeNode = (heap_blk*)(freeNode->next_Addr); //cast U32 as a heap_blk pointer to iterate through the linked list
	}
	
	//return the bottom chunk of free space as a memory block, reduce free space
	mem_blk = (void*)((U32)freeNode + freeNode->length - BLOCK_SIZE);
	
	//update free space list
	freeNode->length -= BLOCK_SIZE;
	if (freeNode->length == 0 && freeNode != (heap_blk*)HEAP_START_ADDR) {
		temp = (heap_blk*)HEAP_START_ADDR;
		while (freeNode != (heap_blk*)(temp->next_Addr)) {
			temp = (heap_blk*)(temp->next_Addr); 
		}
		temp->next_Addr = freeNode->next_Addr;
	}

	__enable_irq(); //atomic(off);

	return mem_blk;
}
	
int k_release_memory_block(void *memory_block) {
	heap_blk* temp = (heap_blk*)HEAP_START_ADDR;

	__disable_irq(); //atomic(on);

	//check memory block pointer is valid
	if ( (U32)memory_block < HEAP_START_ADDR + 4
		|| (U32)memory_block > HEAP_END_ADDR - BLOCK_SIZE
		// || TODO: 8bit alignment
		){
		return RTX_ERR;
	}
	
	

	//special case: memory block is very top of heap
	if ((U32)memory_block == HEAP_START_ADDR + 4){
		temp->length += BLOCK_SIZE;
	}
	else {
		//find where address of mem block lies on free space list
		while (temp->next_Addr != NULL && temp->next_Addr < (U32)memory_block) {
			temp = (heap_blk*)temp->next_Addr; 
		}
		//if immediately after free space, increase above node
		if ((U32)memory_block == (U32)temp + temp->length) {
			temp->length += (U32)BLOCK_SIZE;
		}
		//else create a node, set "next"
		else {
			((heap_blk*)memory_block)->length = (U32)BLOCK_SIZE;
			((heap_blk*)memory_block)->next_Addr = temp->next_Addr;
			temp->next_Addr = (U32)memory_block;
		}
	}
	//if a free space node immediately follows, delete that one and steal its "next", increase length
	if (temp->next_Addr != NULL && temp->next_Addr == (U32)temp + temp->length) {
		temp->length += ((heap_blk*)(temp->next_Addr))->length;
		temp->next_Addr = ((heap_blk*)(temp->next_Addr))->next_Addr;
	}

	//If we have blocked processes, we can now unblock one.
	if (!is_empty_b()) {
		unblock_and_switch_to_blocked_process();
	}

	__enable_irq(); //atomic(off);
	
	return RTX_OK;
}
