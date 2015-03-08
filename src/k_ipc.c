#include "k_ipc.h"

int k_send_message(int process_id, void *message_envelope) {
	
	/*
	atomic ( on ) ;
	set sender_procid , destination_procid ;
	pcb_t * receiving_proc = get_pcb_from_pid ( receiving_pid ) ;
	enqueue env onto the msg_queue of receiving_proc ;
	if ( receiving_proc->state is BLOCKED_ON_RECEIVE ) {
		set receiving_proc state to ready ;
		rpq_enqueue ( receiving_proc ) ;
	}
	atomic ( off ) ;
	*/
	
}

void *k_receive_message(int *sender_id) {
	int current_process = get_procid_of_current_process();
	
	__disable_irq();
	while (!m_any_messages(g_message_queue, current_process)){//} current_process msg_queue is empty ) {
		get_pcb_pointer_from_process_id(current_process)->m_state = BLOCKED_ON_RECEIVE ;
		release_processor() ;
	}
	__enable_irq();
	return RTX_OK; //todo: is this what we want?
}

/*
The message (in the memory block pointed to by the second parameter) 
will be sent to the destination process (process_id) after the expiration
of the delay (timeout, given in millisecond units).
*/
int k_delayed_send(int process_id, void *message_envelope, int delay) {
	
	if (delay < 0) {
		return RTX_ERR;
	}
	
}
