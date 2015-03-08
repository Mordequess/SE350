#include "k_ipc.h"

/* ----- Global Variables ----- */
//message* g_message_queue;				/* Message queue */

message* message_new(int sender, int destination, msgbuf* envelope) {
	message* m;
	m->sender_id = sender;
	m->destination_id = destination;
	m->message_envelope = envelope;
	m->mp_next = NULL;
	return m;
}

int k_send_message(int destination_id, void *message_envelope) {
	message* m;
	pcb* receiving_proc;	

	__disable_irq(); //atomic(on);
	
	//create message package
	m = message_new(get_procid_of_current_process(), destination_id, (msgbuf *)message_envelope);
	receiving_proc = get_pcb_pointer_from_process_id(destination_id);
	m_enqueue(destination_id, m);

	//check if receiver is waiting
	if (receiving_proc->m_state == BLOCKED_ON_RECEIVE) {
		__enable_irq();
		unblock_and_switch_to_blocked_on_receive_process();
	}

	__enable_irq();
	return RTX_OK; //todo: is this what we want?
}

void* k_receive_message(int *sender_id) {
	message* m;
	int current_process = get_procid_of_current_process();
	__disable_irq();
	while (m_is_empty(current_process)){//} current_process msg_queue is empty ) {
		__enable_irq();
		block_current_process_on_receive();
	}
	m = m_dequeue(current_process);
	*sender_id = m->sender_id;
	__enable_irq();
	return (void *)(m->message_envelope);
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
	return RTX_OK;
}


// ------------QUEUE FUNCTIONS ------------------------------------------------

//Add the node to the tail end of the queue
void m_enqueue(int destination_id, message* element) {
	message* queue;
	pcb* p = get_pcb_pointer_from_process_id(destination_id);
	queue = p->m_queue;

	//check if empty
	if (m_is_empty(destination_id)) {
		p->m_queue = element;
		element->mp_next = NULL;
		return;
	}

	//iterate through to find end
	while (queue->mp_next != NULL) {
		queue = queue->mp_next;
	}

	//insert
	element->mp_next = NULL;
	queue->mp_next = element;
}

//Remove and return a node from the front end of the queue
message* m_dequeue(int destination_id) {
	message* element;
	pcb* p = get_pcb_pointer_from_process_id(destination_id);
	element = p->m_queue;
	
	if (!m_is_empty(destination_id)) {
		p->m_queue = p->m_queue->mp_next;
		element->mp_next = NULL;
	}
	return NULL; //null if nothing to dequeue
}

//Removes a node from the queue, regardless of its position
//The input parameter asks for the contents and not for the queue_node
void m_remove_queue_node(int destination_id, message* element) {
	message* queue;
	pcb* p = get_pcb_pointer_from_process_id(destination_id);
	queue = p->m_queue;

	//compare to first item in queue
	if (queue == element){
		p->m_queue = p->m_queue->mp_next;
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
U32 m_is_empty(int destination_id) {
	pcb* p = get_pcb_pointer_from_process_id(destination_id);
	if (p->m_queue == NULL) {
		return 1;
	} else {
		return 0;
	}
}

/*
U32 m_any_messages_from_sender(int destination_id, int sender_id) {
	message* element;
	pcb* p = get_pcb_pointer_from_process_id(destination_id);
	element = p->m_queue;
		
	if (!m_is_empty(destination_id)) {
		while (element->sender_id != sender_id) {
			if (element->mp_next == NULL) return 0;
			else element = element->mp_next;
		}
		return 1;
	}
	else return 0;
}
*/

