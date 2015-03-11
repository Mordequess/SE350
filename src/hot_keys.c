#include "hot_keys.h"
#include "rtx.h"
#include "k_rtx.h"
#include "uart_polling.h"
//#include "printf.h"
#include "k_process.h"

/*
Prints the correct queue depending on the hotkey inputted
If hotkey is invalid, nothing will happen.
*/
void process_hot_key(char c) {
	
	switch(c) {
		case DEBUG_HOTKEY_1:
			uart0_put_string("========== READY QUEUE ==========\n\r");
			print_queue(get_ready_queue());
			uart0_put_string("====== END OF READY QUEUE =======\n\r");
			break;
		case DEBUG_HOTKEY_2:
			uart0_put_string("========== BLOCKED ON MEMORY QUEUE ==========\n\r");
			print_queue(get_blocked_on_memory_queue());
			uart0_put_string("====== END OF BLOCKED ON MEMORY QUEUE =======\n\r");
			break;
		case DEBUG_HOTKEY_3:
			uart0_put_string("========== BLOCKED ON RECEIVE QUEUE ==========\n\r");
			print_queue(get_blocked_on_receive_queue());
			uart0_put_string("====== END OF BLOCKED ON RECEIVE QUEUE =======\n\r");
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
	uart0_put_string("Process Id = ");
	uart0_put_char('0'+ proc->m_pid);
	uart0_put_string(" Priority = ");
	uart0_put_char('0' + proc->m_priority);
	uart0_put_string("\n\r");
}
