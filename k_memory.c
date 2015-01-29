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
					|		space for more PCBs	    |
          |---------------------------|<--- HEAP_START_ADDR
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|<--- HEAP_END_ADDR
          |	   space for more stacks  |
          |---------------------------|<--- gp_stack 
          |    Proc 2 STACK           |
          |...........................|          
          |    Proc 1 STACK           |
          |                           |
0x10008000+---------------------------+ High Address
*/

/* ----- Global Variables ----- */
//pcb **gp_pcbs;
U32 *gp_stack;	/* The last allocated stack low address. 8 bytes aligned */
				/* The first stack starts at the RAM high address */
				/* stack grows down. Fully decremental stack */

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

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

void memory_init(void)
{
	//p_end is going to point to the end of the heap once all is allocated
	unsigned char *p_end = (unsigned char *)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;
	U32 *p1_sp;
	U32 *p2_sp;
  heap_blk* heap_Head;
	
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	// 6 since there are 6 test processes
	gp_pcbs = (pcb **)p_end;
	p_end += 6 * sizeof(pcb *);
  
	//TODO:
	//initialize queues here
	
	//6 again for number of test processes
	for ( i = 0; i < 2; i++ ) {
		gp_pcbs[i] = (pcb *)p_end;
		p_end += sizeof(pcb); 
	}
#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
#endif
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	
	p1_sp = alloc_stack(512);
	p2_sp = alloc_stack(512);
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
  
	//allocate memory for heap
	heap_Head = (heap_blk*)HEAP_START_ADDR;
	heap_Head->next_Addr = NULL;
	heap_Head->length = HEAP_END_ADDR - HEAP_START_ADDR - sizeof(heap_blk *); //length in heap_Head adjusts for header, others won't
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
			//TODO:
			//put PCB on blocked_resource_q ;
			//set process state to BLOCKED_ON_RESOURCE ;
			//release_processor ( ) ;
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

	//TODO:
	//if ( memory block pointer is not valid )
	//	return ERROR_CODE ;

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


	//TODO:
	//if ( "blocked on resource" q not empty ) {
	//	handle_process_ready ( pop ( blocked resource q ) ) ;
	//}

	__enable_irq(); //atomic(off);
	
	return 0;
}


//release_processor
//get/set priority
//priorty queue to hold PCBs
//blocked "queue", we insert into the middle
//can be put on the heap
