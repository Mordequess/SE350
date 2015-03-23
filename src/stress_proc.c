#include "stress_proc.h"
#include "rtx.h"
#include "k_ipc.h"
#include "k_iprocess.h"
#include "util.h"

message* local_queue;

message* local_message_new(msgbuf* envelope) {
	
	message* m = (message *)((U32)envelope + sizeof(msgbuf));
	m->message_envelope = envelope;
	//m->sender_id = sender;
	//m->destination_id = destination;
	//m->expiry_time = get_system_time() + delay;
	m->mp_next = NULL;
	return m;
}

void local_enqueue(message* element) {
	message* temp = local_queue;
	
	//check if empty
	if (local_queue == NULL) {
		local_queue = element;
		element->mp_next = NULL;
		return;
	}

	//iterate through to find end
	while (temp->mp_next != NULL) {
		temp = temp->mp_next;
	}

	//insert
	element->mp_next = NULL;
	temp->mp_next = element;
}

//Remove and return a node from the front end of the queue
message* local_dequeue() {
	message* element = local_queue;
	
	if (local_queue != NULL) {
		local_queue = local_queue->mp_next;
		element->mp_next = NULL;
		return element;
	}
	return NULL; //null if nothing to dequeue
}

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
	message* wrapper;
	msgbuf* current_message;
	msgbuf* hibernation_message;
	int sender_id;
	local_queue = NULL;
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
				copy_string("Process C\n\r", current_message->mtext);
				current_message->mtype = CRT_DISP;
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
						while(local_queue != NULL) {
							send_message(PID_C, local_dequeue()->message_envelope);
						}
						break;
					} else {
						wrapper = local_message_new(current_message);
						local_enqueue(wrapper);
						//TODO: put message on local queue for later processing
					}
				}
			}
				
			else {
				release_memory_block(current_message);
			}
		}
		
		release_processor();
	}
}
