#include "stress_proc.h"
#include "rtx.h"
#include "k_ipc.h"
#include "k_iprocess.h"
#include "util.h"

void stress_proc_a() {
	
	msgbuf* block;
	int sender_id;
	int num;
	
	//Register the %Z command
	msgbuf* registration_message = request_memory_block();
	registration_message->mtype = KCD_REG;
	copy_string("%Z", registration_message->mtext);
	send_message(PID_KCD, registration_message);
	
	while(1) {
		
		block = receive_message(&sender_id);
		if (str_len(block->mtext) >= 2 && block->mtext[0] == '%' && block->mtext[1] == 'Z') {
			release_memory_block(block);
			break; //exit this while loop
		} else {
			release_memory_block(block);
		}
		
	}
	
	num = 0;
	
	while(1) {
		block = request_memory_block();
		block->mtype = COUNT_REPORT;
		block->mtext[0] = num;
		send_message(PID_B, block);
		num++;
		release_processor();
	}
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
	
	//initialize a local message queue
	//...
	// probably just use memory given at compile time.
	lmsg* local_q_head = NULL;
	lmsg* local_q_tail = NULL;
	lmsg* temp = NULL;
	// use space in each memory block - the lmsg* struct lives after 50B
	// arbitrary
	
	while(1) {
		
		if ((local_q_head == NULL)&&(local_q_tail == NULL)) {
			current_message = receive_message(&sender_id);

			temp = (lmsg *)(current_message + 50*sizeof(char)); // offset of 50B
			temp->content = current_message;
			temp->next = NULL;
			local_q_head = temp;
			local_q_tail = local_q_head;
			// case where the queue was empty
		}
		else {
			/* current_message is dequeued off the queue */
			temp = local_q_head;
			local_q_head = local_q_head->next;
			if(local_q_head == NULL) {
				local_q_tail = NULL; // emptied out
			}
			current_message = temp->content;
			temp->next = NULL; // necessary? perhaps not
		}
		
		
		if (current_message->mtype == COUNT_REPORT) {
			if (current_message->mtext[0] % 20 == 0) {
				
				//Send "Process C" to CRT with the current_message envelope
				copy_string("Process C", current_message->mtext);
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
						temp = (lmsg *)(current_message + 50*sizeof(char)); // offset of 50B
						temp->content = current_message;
						temp->next = NULL;
						local_q_tail->next = temp;
						local_q_tail = temp;
					}
				}
			}		
		}
		
		release_memory_block(current_message);
		release_processor();
	}
}
