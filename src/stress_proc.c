#include "stress_proc.h"
#include "rtx.h"
#include "k_ipc.h"
#include "k_iprocess.h"
#include "util.h"

void stress_proc_a() {
	
	
}

/*
 * Receives messages and sends them all to C
 */
void stress_proc_b() {
	
	int sender_id;
	msgbuf* message;
	
	while(1) {
		
		message = receive_message(&sender_id);
		send_message(PID_C, message);
		
	}
	
}

void stress_proc_c() {
	
	msgbuf* current_message;
	msgbuf* hibernation_message;
	int sender_id;
	
	int null_terminator_offset = str_len("Process C");
	
	//initialize a local message queue
	//...
	
	while(1) {
		
		//if (local message queue is empty)
			current_message = receive_message(&sender_id);
		//else
			//current_message is dequeued off the queue
		//endif
		
		
		if (current_message->mtype == COUNT_REPORT) {
			if (current_message->mtext[0] % 20 == 0) {
				
				//Send "Process C" to CRT with the current_message envelope
				copy_string("Process C", current_message->mtext);
				current_message->mtext[null_terminator_offset] = '\0';
				send_message(PID_CRT, current_message);
				
				//hibernate for 10s (send delayed message to itself)
				hibernation_message = request_memory_block();
				hibernation_message->mtype = WAKEUP10;
				hibernation_message->mtext[0] = '\0';
				delayed_send(PID_C, hibernation_message, 10*1000);
				
				//Wait to exit hibernation
				while(1) {
					current_message = receive_message(&sender_id);
					if (current_message->mtype == WAKEUP10) {
						break;
					} else {
						//TODO: put message on local queue for later processing
					}
				}
			}		
		}
		
		release_memory_block(current_message);
		release_processor();
	}
	
	
}

