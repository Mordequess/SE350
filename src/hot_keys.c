#include "hot_keys.h"
#include "rtx.h"
#include "k_rtx.h"
#include "uart_polling.h"
#include "printf.h"

void process_hot_key(char c) {
	
	#ifdef _DEBUG_HOTKEYS
	
	switch(c) {
		case DEBUG_HOTKEY_1:
			//print procs on ready queue
			print_queue();
			break;
		case DEBUG_HOTKEY_2:
			//print procs on blocked on memory queue
			print_queue();
			break;
		case DEBUG_HOTKEY_3:
			print_queue();
			//print procs on blocked on receive queue
			break;
	}
	
	#endif
	
}

void print_queue() {
	
}

void print_process(pcb *proc) {
	
	printf("Process Id = %d, Priority = %d\r\n", proc->m_pid, proc->m_priority);
	
}
