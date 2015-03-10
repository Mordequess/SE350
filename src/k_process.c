#include "k_process.h"
#include "k_iprocess.h"
#include "k_system_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
pcb **gp_pcbs;                  /* array of pcbs */
pcb *gp_current_process = NULL; /* always point to the current RUN process */
pcb* g_ready_queue;				/* Ready queue */
pcb* g_blocked_on_memory_queue;			/* Blocked queue */
pcb* g_blocked_on_receive_queue;

/* process initialization table */
static PROC_INIT g_proc_table[NUM_PROCESSES];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];


//Null process
void null_process() {
	while (1) {
		release_processor();
	}
}

/**
 * @brief: initialize all processes in the system
 */
void process_init() 
{
	int i;
	U32 *sp;
	  
  /* fill out the initialization table */
	set_test_procs();

	//NULL process
	g_proc_table[PID_NULL].m_pid = PID_NULL;
	g_proc_table[PID_NULL].m_stack_size = STACK_SIZE;
	g_proc_table[PID_NULL].mpf_start_pc = &null_process;
	g_proc_table[PID_NULL].m_priority = NULL_PRIORITY;
	
	//Timer iprocess
	g_proc_table[PID_TIMER].m_pid = PID_TIMER;
	g_proc_table[PID_TIMER].m_stack_size = STACK_SIZE;
	g_proc_table[PID_TIMER].mpf_start_pc = &timer_i_process;
	g_proc_table[PID_TIMER].m_priority = HIGH;
	
	//Uart iprocess
	g_proc_table[PID_UART].m_pid = PID_UART;
	g_proc_table[PID_UART].m_stack_size = STACK_SIZE;
	g_proc_table[PID_UART].mpf_start_pc = &uart_i_process;
	g_proc_table[PID_UART].m_priority = HIGH;
	
	//KCD process (always HIGH)
	g_proc_table[PID_KCD].m_pid = PID_KCD;
	g_proc_table[PID_KCD].m_stack_size = STACK_SIZE;
	g_proc_table[PID_KCD].mpf_start_pc = &kcd_proc;
	g_proc_table[PID_KCD].m_priority = HIGH;
	
	//CRT process (always HIGH)
	g_proc_table[PID_CRT].m_pid = PID_CRT;
	g_proc_table[PID_CRT].m_stack_size = STACK_SIZE;
	g_proc_table[PID_CRT].mpf_start_pc = &crt_proc;
	g_proc_table[PID_CRT].m_priority = HIGH;
	
	//Wall clock process (always HIGH)
	g_proc_table[PID_WALL_CLOCK].m_pid = PID_WALL_CLOCK;
	g_proc_table[PID_WALL_CLOCK].m_stack_size = STACK_SIZE;
	g_proc_table[PID_WALL_CLOCK].mpf_start_pc = &wall_clock_proc;
	g_proc_table[PID_WALL_CLOCK].m_priority = HIGH;

	//For the six test processes
	for (i = 1; i <= NUM_TEST_PROCS; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i-1].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i-1].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i-1].mpf_start_pc;
		g_proc_table[i].m_priority = g_test_procs[i-1].m_priority;
	}
  
	/* initilize exception stack frame (i.e. initial context) for each process */
	for (i = 0; i < NUM_PROCESSES; i++ ) {
		int j;
		
		//avoid modifying procs that don't appear in P1 and P2
		if (i > PID_P6 && i < PID_WALL_CLOCK) {
			continue;
		}

		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_queue = NULL;
		
		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
	}

	//set up ready queue
	//It contains all test processes as well as null, wall, kcd, and crt
	//note: does not change state, they all still count as NEW
	for ( i = 0; i < NUM_PROCESSES; i++ ) {
		if ((i <= PID_P6) || (i == PID_WALL_CLOCK) || (i == PID_KCD) || (i == PID_CRT)) {
			enqueue(&g_ready_queue, gp_pcbs[i]);
		}
	}
	
#ifdef DEBUG_0  
	printf("--PROCESS DEBUG STATUS--\n");
	for(i = 0; i < NUM_PROCESSES; i++){
		printf("PCB_%x stack pointer: %x\n", i, gp_pcbs[i]->mp_sp);
	}
	printf("--END OF PROCESS DEBUG STATUS--\n");
#endif
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
pcb *scheduler(void){
	pcb* next;
	//If process is running, find something to swap it with
	if (gp_current_process != NULL) {
		if (gp_current_process->m_state == READY) {
			enqueue(&g_ready_queue, gp_current_process);
		}
		else if (gp_current_process->m_state == BLOCKED_ON_RECEIVE) {
			enqueue(&g_blocked_on_receive_queue, gp_current_process);
		}	
		else enqueue(&g_blocked_on_memory_queue, gp_current_process);
	}
	
	next = dequeue(&g_ready_queue); //if none are ready, defaults to null process
	return next;
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
int process_switch(pcb *p_pcb_old) {

	PROC_STATE_E state;
	pcb* test_test = gp_current_process;
	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			//p_pcb_old->m_state = READY;
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		gp_current_process->m_state = RUNNING;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	}
	
	//The following will only execute if the if block above is FALSE 

	if (gp_current_process != p_pcb_old) {
		if (state == READY){ 		
			//p_pcb_old->m_state = READY; 
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			gp_current_process->m_state = RUNNING;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
		} else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	else {
		gp_current_process->m_state = RUNNING;
	}
	return RTX_OK;
}

/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void){
	
	pcb *p_pcb_old = gp_current_process; // initially this is NULL
	
	if (p_pcb_old != NULL && p_pcb_old->m_state != BLOCKED_ON_MEMORY && p_pcb_old->m_state != BLOCKED_ON_RECEIVE) {
		p_pcb_old->m_state = READY;
	}
	
	gp_current_process = scheduler();
	
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
	
	pcb_modified_process->m_priority = priority;

	//Since priority has changed, we need to move the process to another queue.
	//There will be different queues depending on whether it is ready/new, blocked, or the active process
	if (pcb_modified_process->m_state == READY || pcb_modified_process->m_state == NEW) {
		remove_queue_node(&g_ready_queue, pcb_modified_process);
		enqueue(&g_ready_queue, pcb_modified_process);
	} else if (pcb_modified_process->m_state == BLOCKED_ON_MEMORY) {
		remove_queue_node(&g_blocked_on_memory_queue, pcb_modified_process);
		enqueue(&g_blocked_on_memory_queue, pcb_modified_process);
	} else if (pcb_modified_process->m_state == BLOCKED_ON_RECEIVE) {
		remove_queue_node(&g_blocked_on_receive_queue, pcb_modified_process);
		enqueue(&g_blocked_on_memory_queue, pcb_modified_process);
	} /*else if (pcb_modified_process->m_state == RUNNING) {
		//enqueue(&g_ready_queue, pcb_modified_process);
		//pcb_modified_process->m_state = READY;
		
		//no action required
	}*/
	
	//Since priority was modified, we need to pre-empt
	k_release_processor();
	
	return RTX_OK;
}

//Priority getter
int k_get_process_priority(int process_id) {
		
	pcb* p_pcb_param = get_pcb_pointer_from_process_id(process_id);
	
	//For invalid process_id out of range, return RTX_ERR
	//For P2, valid processes are 0-6 and then 11-15
	if (process_id < PID_NULL || process_id > PID_UART || (process_id > PID_P6 && process_id < PID_WALL_CLOCK)) {
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

pcb *get_current_process() {
	return gp_current_process;
}

pcb* get_ready_queue() {
	return g_ready_queue;
}

pcb* get_blocked_on_memory_queue() {
	return g_blocked_on_memory_queue;
}

pcb* get_blocked_on_receive_queue() {
	return g_blocked_on_receive_queue;
}

pcb *get_pcb_pointer_from_process_id(int process_id) {
	
	//invalid process_id check
	if (process_id < PID_NULL || process_id > PID_UART) {
		return NULL;
	}
	
	return gp_pcbs[process_id];
}

//called from memory
void block_current_process_on_memory(void) {
	gp_current_process->m_state = BLOCKED_ON_MEMORY;
	k_release_processor();
}

void block_current_process_on_receive(void) {
	gp_current_process->m_state = BLOCKED_ON_RECEIVE;
	k_release_processor();
}

//Tells processor to switch to the highest blocked-on-memory process. It is no longer blocked.
int unblock_and_switch_to_blocked_on_memory_process(void) {
	pcb* processToSwitchTo = dequeue(&g_blocked_on_memory_queue);
	processToSwitchTo->m_state = READY;
	enqueue(&g_ready_queue, processToSwitchTo);
	
	gp_current_process->m_state = READY;
	k_release_processor();
	return RTX_OK;
}

//Tells processor to switch to the highest blocked-on-memory process. It is no longer blocked.
int unblock_and_switch_to_blocked_on_receive_process(pcb* processToSwitchTo) {
	remove_queue_node(&g_blocked_on_receive_queue, processToSwitchTo);
	processToSwitchTo->m_state = READY;
	enqueue(&g_ready_queue, processToSwitchTo);
	
	gp_current_process->m_state = READY;
	k_release_processor();
	return RTX_OK;
}

int get_procid_of_current_process(){
	return gp_current_process->m_pid;
}




// ------------QUEUE FUNCTIONS ------------------------------------------------

//Add the node to the tail end of the queue
void enqueue(pcb** targetQueue, pcb* element) {
	pcb* queue = *targetQueue;

	//check if empty
	if (is_empty(*targetQueue)) {
		*targetQueue = element;
		element->mp_next = NULL;
		return;
	}

	//compare to first item in queue
	if (element->m_priority < (*targetQueue)->m_priority){
		element->mp_next = *targetQueue;
		*targetQueue = element;
		return;
	}

	//iterate through to find where to insert
	while (queue->mp_next != NULL && element->m_priority >= queue->mp_next->m_priority) {
		queue = queue->mp_next;
	}

	//insert
	element->mp_next = queue->mp_next;
	queue->mp_next = element;
}

//Remove and return a node from the front end of the queue
pcb* dequeue(pcb** targetQueue) {
	if (!is_empty(*targetQueue)) {
		pcb* element = *targetQueue;
		*targetQueue = (*targetQueue)->mp_next;
		element->mp_next = NULL;
		return element;
	}
	return NULL; //null if nothing to dequeue
}

//Removes a node from the queue, regardless of its position
//The input parameter asks for the contents and not for the queue_node
void remove_queue_node(pcb** targetQueue, pcb* element) {
	pcb* queue = *targetQueue;

	//compare to first item in queue
	if (*targetQueue == element){
		*targetQueue = (*targetQueue)->mp_next;
		return;
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
U32 is_empty(pcb* targetQueue) {
	if (targetQueue == NULL) {
		return 1;
	} else {
		return 0;
	}
}
