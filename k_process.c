/**
 * @file:   k_process.c  
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS. 
 *       The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations. 
 */

#include "k_process.h"


#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
pcb **gp_pcbs;                  /* array of pcbs */
pcb *gp_current_process = NULL; /* always point to the current RUN process */

queue g_ready_queue[NUM_PRIORITIES];	/* Ready queue */
queue g_blocked_queue[NUM_PRIORITIES];	/* Blocked queue */

queue_node* g_queue_nodes[NUM_PROCESSES];

/* process initialization table */
PROC_INIT g_proc_table[NUM_TEST_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
void process_init() 
{
	int i;
	U32 *sp;
	
	//TODO: need to initialize null process and add to gp_pcb
	//~ local variables to help with debugging
  
  /* fill out the initialization table */
	set_test_procs();
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
	}
  
	/* initilize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		int j;
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		
		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
	}
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */

pcb *scheduler(void)
{
	
	//No process currently running
	if (gp_current_process == NULL) {
		return gp_pcbs[0];
		//~return first process pcb - right now, this is testproc 1
	}

	//If process is running, find something to swap it with
	//...
	
	//~ we should probably return the pcb for the null process, really
	//return NULL if error happens.
	return NULL;
}

/*
 *@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(pcb *p_pcb_old) 
{
	//THE CODE BELOW IS THE GITHUB CODE.
	pcb *curr = gp_current_process; // for debug
	PROC_STATE_E state;
	
	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			p_pcb_old->m_state = READY;
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		gp_current_process->m_state = RUNNING;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	//The following will only execute if the if block above is FALSE 

	if (gp_current_process != p_pcb_old) {
		if (state == READY){ 		
			p_pcb_old->m_state = READY; 
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			gp_current_process->m_state = RUNNING;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
		} else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	return RTX_OK;
}
/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	/* GITHUB CODE
1. Set current process to state ready
2. rpq enqueue(current process) put current process in ready queues
3. process switch invokes scheduler and context-switches to the
new process
	*/
	pcb *curr = gp_current_process; //for debug
	pcb *p_pcb_old = gp_current_process; // initially this is NULL
	gp_current_process = scheduler(); // this now becomes pcbs[0]
	
	//If scheduler returned NULL, we have an error.
	//Keep currently running process the same.
	if (gp_current_process == NULL) {
		gp_current_process = p_pcb_old;
		//return RTX_ERR;
	}
	
	//If no process was running to begin with, set to current
	if (p_pcb_old == NULL) {
		p_pcb_old = gp_current_process;
	}
	
	//Switch process. The function knows both the old and new process
	process_switch(p_pcb_old);
	return RTX_OK;
	
}

//Null process
void null_process() {
	while (1) {
		printf("Inside null process");
		k_release_processor();
	}
}

//Priority setter
int set_process_priority(int process_id, int priority) {
	
	pcb* pcb_modified_process = get_pcb_pointer_from_process_id(process_id);
	U32 oldPriority = pcb_modified_process->m_priority;
	
	//Valid priority values are {0, 1, 2, 3}, with 3 being LOWEST
	if (priority < HIGH || priority > LOWEST) {
		return RTX_ERR;
	}
	
	//Only processes 1-6 can have their priorities set
	if (process_id < PID_P1 || process_id > PID_P6) {
		return RTX_ERR;
	}
	
	//Null check on pointer
	if (pcb_modified_process == NULL) {
		return RTX_ERR;
	}
	
	//Since priority has changed, we need to move the process to another queue.
	//There will be different queues depending on whether it is ready or blocked
	if (pcb_modified_process->m_state == READY) {
		queue_node *processNode = remove_queue_node(&g_ready_queue[oldPriority], pcb_modified_process);
		enqueue(&g_ready_queue[priority], processNode);
	} else if (pcb_modified_process->m_state == BLOCKED) {
		queue_node *processNode = remove_queue_node(&g_blocked_queue[oldPriority], pcb_modified_process);
		enqueue(&g_blocked_queue[priority], processNode);
	}
	
	pcb_modified_process->m_priority = priority;
	
	//Since priority was modified, we may need to pre-empt
	//If a ready process now has higher priority than the current one, then release processor
	if (is_a_more_important_process_ready(pcb_modified_process)) {
		k_release_processor();
	}
	
	return RTX_OK;
}

//Priority getter
int get_process_priority(int process_id) {
		
	pcb* p_pcb_param = get_pcb_pointer_from_process_id(process_id);
	
	//For invalid process_id out of range, manual says to return -1 (RTX_ERR)
	if (process_id > PID_P6 || process_id < PID_P1) {
		return RTX_ERR;
	}

	//Null check on pointer
	if (p_pcb_param == NULL) {
		return RTX_ERR;
	}
	
	return p_pcb_param->m_priority;
}

// ------------------- API ENDS HERE ------------------------------------------

// ----------- HELPER FUNCTIONS BEGIN HERE ------------------------------------

pcb *get_pcb_pointer_from_process_id(int process_id) {
	
	//invalid process_id check
	if (process_id < 0 || process_id > 6) {
		return NULL;
	}
	
	return gp_pcbs[process_id];
}

//Returns the ready process highest on the priority queue
pcb* get_next_ready_process(void) {
	int i;
	for (i = 0; i < NUM_PRIORITIES; i++) {
		if (!is_empty(&g_ready_queue[i])) {
			queue_node *procNode = g_ready_queue[i].head;
			return (pcb*)procNode->contents;
		}
	}
	
	//Should never get down here
	//If nothing is ready, null process should run once we reach priority 4.
}

//Returns the highest-priority process that is currently blocked on memory
pcb* get_next_blocked_process(void) {
	int i;
	for (i = 0; i < NUM_PRIORITIES; i++) {
		if (!is_empty(&g_blocked_queue[i])) {
			queue_node *procNode = g_blocked_queue[i].head;
			return (pcb*)procNode->contents;
		}
	}
	
	//No blocked processes anywhere. Just return null.
	return NULL;
}

//Returns 1 if there exists a ready process more important
// than the one currently running.
U32 is_a_more_important_process_ready(pcb* currentProcess) {
	U32 currentPriority = currentProcess->m_priority;
	
	int i;
	for (i = 0; i < currentPriority; i++) {
		if (!is_empty(&g_ready_queue[i])) {
			return 1;
		}
	}
	return 0;
}

//For a given PCB, returns its associated queue_node
queue_node* get_queue_node_for_process(pcb* proc) {
	
	int pid = proc->m_pid;
	
	if (pid >= 0 && pid <= 6) {
		return g_queue_nodes[pid];
	} else {
		return NULL;
	}
	
}

void block_current_process(void) {
	gp_current_process->m_state = BLOCKED;
	enqueue(&g_blocked_queue[gp_current_process->m_priority], get_queue_node_for_process(gp_current_process));
	k_release_processor();
}

//Tells processor to switch to the highest blocked process. It is no longer blocked.
U32 unblock_and_switch_to_blocked_process(void) {
	
	pcb* processToSwitchTo;
	pcb* processToSwitchOutOf;
	queue_node *removedNode;
	
	processToSwitchTo = get_next_blocked_process();
	processToSwitchOutOf = gp_current_process;
	
	//Does not dequeue. Do a null check first
	if (processToSwitchTo == NULL) {
		return RTX_ERR; //no blocked processes. This function should not have been called
	}
	
	//remove from queue
	removedNode = dequeue(&g_blocked_queue[processToSwitchTo->m_priority]);
	
	//The chosen process and the dequeued one should be the same.
	if (removedNode->contents != processToSwitchTo) {
		return RTX_ERR;
	}
	
	//set as next process to run, bypassing the scheduler()
	gp_current_process = processToSwitchTo;
	
	process_switch(processToSwitchOutOf);
	
	return RTX_OK;
}
