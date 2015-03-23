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
			uart1_put_string("====== READY QUEUE ==============\n\r");
			print_queue(get_ready_queue());
			uart1_put_string("====== END OF READY QUEUE =======\n\r");
			break;
		case DEBUG_HOTKEY_2:
			uart1_put_string("====== BLOCKED ON MEMORY QUEUE ==============\n\r");
			print_queue(get_blocked_on_memory_queue());
			uart1_put_string("====== END OF BLOCKED ON MEMORY QUEUE =======\n\r");
			break;
		case DEBUG_HOTKEY_3:
			uart1_put_string("====== BLOCKED ON RECEIVE QUEUE ==============\n\r");
			print_queue(get_blocked_on_receive_queue());
			uart1_put_string("====== END OF BLOCKED ON RECEIVE QUEUE =======\n\r");
			break;
		case DEBUG_HOTKEY_4:
			uart1_put_string("====== RUNNING PROCESS ==============\n\r");
			print_process(get_current_process());
			uart1_put_string("====== END OF RUNNING PROCESS =======\n\r");
			
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
	uart1_put_string("Process Id = ");
	if (proc->m_pid > 9) uart1_put_char('0'+ proc->m_pid / 10);
	uart1_put_char('0'+ proc->m_pid % 10);
	uart1_put_string(" Priority = ");
	if (proc->m_priority > 9) uart1_put_char('0'+ proc->m_priority / 10);
	uart1_put_char('0' + proc->m_priority % 10);
	uart1_put_string("\n\r");
}
