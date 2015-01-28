#include "rtx.h"

//Add the node to the tail end of the queue
void enqueue(queue* process_queue, queue_node* new_node) {
	
	queue_node *prevTail = (*process_queue).tail;
	(*process_queue).tail = new_node;
	(*new_node).next = NULL;
	
	//If there is a tail, set its next value
	if (prevTail != NULL) {
		(*prevTail).next = new_node;
	}
	
	//If queue is empty, set the new node to be the head too
	if ((*process_queue).head == NULL) {
		(*process_queue).head = new_node;
	}
	
}

//Remove and return a node from the front end of the queue
queue_node* dequeue(queue* process_queue) {
	
	queue_node *headNode = (*process_queue).head;
	
	//Queue is empty. Nothing to pop.
	//Callers need to have null checks to make sure they got something
	if (headNode == NULL) {
		return NULL;
	}
	
	(*process_queue).head = (*headNode).next;
	(*headNode).next = NULL;
	
	//If queue is now empty, tail should be set to NULL as well
	if ((*process_queue).head == NULL) {
		(*process_queue).tail = NULL;
	}
	
	return headNode;
}

//Emptiness check.
//Will return 1 if empty and 0 if not.
U32 is_empty(queue* process_queue) {
	
	if ((*process_queue).head != NULL && (*process_queue).tail != NULL) {
		return 0;
	} else {
		return 1;
	}
	
}
