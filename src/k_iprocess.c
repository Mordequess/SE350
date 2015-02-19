#include "k_iprocess.h"

/*
The message (in the memory block pointed to by the second parameter) 
will be sent to the destination process (process_id) after the expiration
of the delay (timeout, given in millisecond units).
*/
int k_delayed_send(int process_id, void *message_envelope, int delay) {
	
	
}


void timer_i_process() {
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
}


void uart0_i_process() {
	
}

void init_i_processes() {
	
}
