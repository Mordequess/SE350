#include "k_iprocess.h"
#include "rtx.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

#ifdef _DEBUG_HOTKEYS
#include "printf.h"
#endif 

void timer_i_process() {
	
	__disable_irq();
	
	/*
	// get pending requests
	while ( pending messages to i-process ) {
		insert envelope into the timeout queue ;
	}
	while ( first message in queue timeout expired ) {
		msg_t * env = dequeue ( timeout_queue ) ;
		int target_pid = env->destination_pid ;
		// forward msg to destination
		send_message ( target_pid , env ) ;
	}
	*/
	
	
	__enable_irq();
}


void uart0_i_process() {
	
	__disable_irq();
	
	
	
	
	#ifdef _DEBUG_HOTKEYS
	
	
	
	
	#endif
	
	
	
	
	__enable_irq();
	
}

/*
 initializes the uart and timer iprocesses (pcbs, stack space etc)
 */
void init_i_processes() {
	
	int i;
	U32 *sp;

	//NULL process
	/*
	g_proc_table[0].m_pid = 0;
	g_proc_table[0].m_stack_size = STACK_SIZE;
	g_proc_table[0].mpf_start_pc = &null_process;
	g_proc_table[0].m_priority = NULL_PRIORITY;
*/
	/*
	for (i = 1; i < NUM_PROCESSES; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i-1].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i-1].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i-1].mpf_start_pc;
		g_proc_table[i].m_priority = g_test_procs[i-1].m_priority;
	}
  */
	/* initilize exception stack frame (i.e. initial context) for each process */
	
	/*
	for (i = 0; i < NUM_PROCESSES; i++ ) {
		int j;
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->m_state = NEW;
		
		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
	}
	*/
	
}
