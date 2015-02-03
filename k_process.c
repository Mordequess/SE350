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
pcb* g_ready_queue;				/* Ready queue */
pcb* g_blocked_queue;			/* Blocked queue */

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

	//set up ready queue with all processes
	//note: does not change state, they all still count as NEW
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		enqueue(g_ready_queue, gp_pcbs[i]);
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
	//for loop through gp_pcbs looking for new processes
	//else 

	//No process currently running (startup only)
	if (gp_current_process == NULL) {
		return gp_pcbs[0];
		//~return first process pcb - right now, this is testproc 1
	}
	else if (gp_current_process == gp_pcbs[0]){
		return gp_pcbs[1];
	}
	else return gp_pcbs[0];

	//TODO:
	//If process is running, find something to swap it with
	//gp_current_process->m_state = READY;
	//pbc* next = g_ready_queue; //if none are ready, defaults to null process
	//dequeue(get_queue_node_for_process(next));
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
		release_processor();
	}
}

//Priority setter
int k_set_process_priority(int process_id, int priority) {
	
	pcb* pcb_modified_process = get_pcb_pointer_from_process_id(process_id);
	
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
		remove_queue_node(g_ready_queue, pcb_modified_process);
		enqueue(g_ready_queue, pcb_modified_process);
	} else if (pcb_modified_process->m_state == BLOCKED) {
		remove_queue_node(g_blocked_queue, pcb_modified_process);
		enqueue(g_blocked_queue, pcb_modified_process);
	}
	
	pcb_modified_process->m_priority = priority;
	
	//Since priority was modified, we may need to pre-empt
	//If a ready process now has higher priority than the current one, then release processor
	if (is_a_more_important_process_ready(pcb_modified_process)) {
		release_processor(); //TEMP: changing this from k_release to release out of fear
	}
	
	return RTX_OK;
}

//Priority getter
int k_get_process_priority(int process_id) {
		
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

// ------------QUEUE FUNCTIONS ------------------------------------------------

//Add the node to the tail end of the queue
void enqueue(pcb* queue_Fixed, pcb* element) {
	pcb* queue = queue_Fixed;

	//check if empty
	if (is_empty(queue_Fixed)) {
		queue_Fixed = element;
		element->mp_next = NULL;
		return;
	}

	//compare to first item in queue
	if (queue_Fixed->m_priority < element->m_priority){
		element->mp_next = queue_Fixed;
		queue_Fixed = element;
		return;
	}

	//iterate through to find where to insert
	while (queue->mp_next->m_priority <= element->m_priority) {
		queue = queue->mp_next;
	}

	//insert
	element->mp_next = queue->mp_next;
	queue->mp_next = element;
}

//Remove and return a node from the front end of the queue
pcb* dequeue(pcb* queue) {
	if (!is_empty(queue)) {
		pcb* element = queue;
		queue = queue->mp_next;
		return element;
	}
	return NULL; //null if nothing to dequeue
}

//Removes a node from the queue, regardless of its position
//The input parameter asks for the contents and not for the queue_node
void remove_queue_node(pcb* queue_Fixed, pcb* element) {
	pcb* queue = queue_Fixed;

	//compare to first item in queue
	if (queue_Fixed == element){
		queue_Fixed = queue_Fixed->mp_next;
	}

	//iterate through to find what to remove
	while (queue->mp_next != element) {
		queue = queue->mp_next;
	}

	//remove
	queue->mp_next = element->mp_next;
	element->mp_next = NULL;
}

//Emptiness check.
//Will return 1 if empty and 0 if not.
U32 is_empty(pcb* queue) {
	if (queue == NULL) {
		return 0;
	} else {
		return 1;
	}
}

// ----------- HELPER FUNCTIONS BEGIN HERE ------------------------------------

pcb *get_pcb_pointer_from_process_id(int process_id) {
	
	//invalid process_id check
	if (process_id < 0 || process_id > 6) {
		return NULL;
	}
	
	return gp_pcbs[process_id];
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

//called from memory
void block_current_process(void) {
	gp_current_process->m_state = BLOCKED;
	enqueue(g_blocked_queue, gp_current_process);
	release_processor();
}

//Tells processor to switch to the highest blocked process. It is no longer blocked.
int unblock_and_switch_to_blocked_process(void) {
	
	pcb* processToSwitchTo;
	pcb* processToSwitchOutOf;
	
	processToSwitchOutOf = gp_current_process;
	
	//Does not dequeue. Do a null check first
	if (g_blocked_queue == NULL) {
		return RTX_ERR; //no blocked processes. This function should not have been called
	}
	
	//remove from queue
	processToSwitchTo = dequeue(g_blocked_queue);
	
	//set as next process to run, bypassing the scheduler()
	gp_current_process = processToSwitchTo;
	
	process_switch(processToSwitchOutOf);
	
	return RTX_OK;
}
