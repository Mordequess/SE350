#include "k_ipc.h"

/* ----- Global Variables ----- */
//message* g_message_queue;				/* Message queue */

int k_send_message(int destination_id, void *message_envelope) {
	message* m;
	pcb* receiving_proc;
	
	__disable_irq(); //atomic(on);
	
	//create message package
	m = message_new(get_procid_of_current_process(), destination_id, (msgbuf *)message_envelope);
	receiving_proc = get_pcb_pointer_from_process_id(destination_id);
	m_enqueue(&g_message_queue, m);

	//check if receiver is waiting
	if (receiving_proc->m_state == BLOCKED_ON_RECEIVE) {
		receiving_proc->m_state = READY;
		remove_queue_node(&g_blocked_on_receive_queue, receiving_proc);
		enqueue(&g_ready_queue, receiving_proc);
	}

	__enable_irq();
	return RTX_OK; //todo: is this what we want?
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
	return RTX_OK; //todo: is this what we want?
}

// ------------QUEUE FUNCTIONS ------------------------------------------------

//Add the node to the tail end of the queue
void m_enqueue(message** targetQueue, message* element) {
	message* queue = *targetQueue;

	//check if empty
	if (m_is_empty(*targetQueue)) {
		*targetQueue = element;
		element->mp_next = NULL;
		return;
	}

	//compare to first item in queue
	if (element->destination_id < (*targetQueue)->destination_id){
		element->mp_next = *targetQueue;
		*targetQueue = element;
		return;
	}

	//iterate through to find where to insert
	while (queue->mp_next != NULL && element->destination_id >= queue->mp_next->destination_id) {
		queue = queue->mp_next;
	}

	//insert
	element->mp_next = queue->mp_next;
	queue->mp_next = element;
}

//Remove and return a node from the front end of the queue
message* m_dequeue(message** targetQueue) {
	if (!m_is_empty(*targetQueue)) {
		message* element = *targetQueue;
		*targetQueue = (*targetQueue)->mp_next;
		element->mp_next = NULL;
		return element;
	}
	return NULL; //null if nothing to dequeue
}

//Removes a node from the queue, regardless of its position
//The input parameter asks for the contents and not for the queue_node
void m_remove_queue_node(message** targetQueue, message* element) {
	message* queue = *targetQueue;

	//compare to first item in queue
	if (*targetQueue == element){
		*targetQueue = (*targetQueue)->mp_next;
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
U32 m_is_empty(message* targetQueue) {
	if (targetQueue == NULL) {
		return 1;
	} else {
		return 0;
	}
}

//check if any messages with destination id.
//Will return 1 if empty and 0 if not.
U32 m_any_messages(message* targetQueue, int destination_id) {
	if (targetQueue == NULL) {
		return 1;
	} else {
		return 0;
	}
}
