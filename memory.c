#include "memory.h"

void *request_memory_block(void) {
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
	
	return 0; //compiler warning. Replace later

}
	
int release_memory_block(void *memory_block) {
	
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
	
	return 0; //to compile
}

/* TAKEN FROM SE350 GITHUB */
/* ----- Global Variables ----- */
unsigned int *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

unsigned int *alloc_stack(unsigned int size_b) 
{
	unsigned int *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (unsigned int *)((unsigned char *)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((unsigned int)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}

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

void memory_init(void)
{
	//p_end is going to point to the end of the heap once all is allocated
	unsigned char *p_end = (unsigned char *)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;
	unsigned int *p1_sp;
	unsigned int *p2_sp;
  
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
	
	gp_stack = (unsigned int *)RAM_END_ADDR;
	if ((unsigned int)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
  
	/* allocate memory for heap, not implemented yet*/
	
	// so at this point, gp_stack is pointing to where
	// the heap starts - (right?)
	// and also then, we know that p_end is pointing to where the heap ends
}

/* TAKEN FROM SE350 GITHUB */
