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
