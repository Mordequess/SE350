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
	/*
	atomic ( on ) ;
	while ( current_process msg_queue is empty ) {
		set current_process state to BLOCKED_ON_RECEIVE ;
		release_processor ( ) ;
	}
	msg_t * env = dequeue current_process msg queue ;
	atomic ( off ) ;
	return env ;
	*/
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
