#include "k_memory.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

/**
 * @brief: Initialize RAM as follows:
0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|  
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|          
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address
*/

/* ----- Global Variables ----- */
heap_blk heap_Head;
//pcb **gp_pcbs;
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
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
  
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	// 2 since there are 2 test processes
	gp_pcbs = (pcb **)p_end;
	p_end += 2 * sizeof(pcb *);
  
	//2 again for number of test processes
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
  
	/* allocate memory for heap, not implemented yet*/
	
	//heap fixed start and end address found in memory.h
	heap_Head.start_Addr = HEAP_START_ADDR;
	heap_Head.next_Addr = NULL;
	heap_Head.length = HEAP_START_ADDR - HEAP_END_ADDR;
	
	
	// so at this point, gp_stack is pointing to where
	// the heap starts - (right?)
	// and also then, we know that p_end is pointing to where the heap ends
}
void *k_request_memory_block(void) {
	/* PROVIDED_PSEUDOCODE
	atomic ( on ) ;
	while ( no memory block is available ) {
		put PCB on blocked_resource_q ;
		set process state to BLOCKED_ON_RESOURCE ;
		release_processor ( ) ;
	}
	int mem_blk = next free block ;
	update the heap ;
	atomic ( off ) ;
	return mem_blk ;
	*/
	void* mem_blk;
	
	// BLOCK_SIZE = 0x80 (128)
	
	heap_blk* heap = &heap_Head; //start at the top of the heap by making a pointer to heap_Head
	while (heap->length < BLOCK_SIZE) {
		if (heap->next_Addr == 0) return NULL; //if this is last free block, the request can't be filled
		else heap = (heap_blk*)heap->next_Addr; //cast U32 as heap_blk pointer
	}
	
	mem_blk = (void*)(heap->start_Addr + heap->length - BLOCK_SIZE);
	heap->length -= BLOCK_SIZE;
	
	return mem_blk;
	
	//case: requested block is entire free space block
	//		must remove this free space block from linked list (probably. length 0 will not satisfy and requests)

}
	
int k_release_memory_block(void *memory_block) {
	
	/* PROVIDED PSEUDOCODE
	atomic ( on ) ;
	if ( memory block pointer is not valid )
		return ERROR_CODE ;
	put memory_block into heap ;
	if ( blocked on resource q not empty ) {
		handle_process_ready ( pop ( blocked resource q ) ) ;
	}
	atomic ( off ) ;
	return SUCCESS_CODE ;
	*/
	
	
	
	return 0;
}
