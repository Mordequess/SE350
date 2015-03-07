#include "hot_keys.h"
#include "rtx.h"
#include "k_rtx.h"
#include "uart_polling.h"
#include "printf.h"
#include "k_process.h"

/*
Prints the correct queue depending on the hotkey inputted
If hotkey is invalid, nothing will happen.
*/
void process_hot_key(char c) {
	
	switch(c) {
		case DEBUG_HOTKEY_1:
			printf("========== READY QUEUE ==========\n\r");
			print_queue(get_ready_queue());
			printf("====== END OF READY QUEUE =======\n\r");
			break;
		case DEBUG_HOTKEY_2:
			printf("========== BLOCKED ON MEMORY QUEUE ==========\n\r");
			print_queue(get_blocked_on_memory_queue());
			printf("====== END OF BLOCKED ON MEMORY QUEUE =======\n\r");
			break;
		case DEBUG_HOTKEY_3:
			printf("========== BLOCKED ON RECEIVE QUEUE ==========\n\r");
			print_queue(get_blocked_on_receive_queue());
			printf("====== END OF BLOCKED ON RECEIVE QUEUE =======\n\r");
			break;
	}
}

/*
Prints a queue given a head.
This process must not modify the queue itself
*/
void print_queue(pcb* head) {
	
	pcb* temp = head;
	if (head == NULL) return;

	while (temp != NULL) {
		print_process(temp);
		temp = temp->mp_next;
	}
}

/*
Prints the PID and priority of a process.
*/
void print_process(pcb *proc) {
	printf("Process Id = %d, Priority = %d\n\r", proc->m_pid, proc->m_priority);
}
