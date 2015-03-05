#include "k_iprocess.h"
#include "rtx.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

#ifdef _DEBUG_HOTKEYS
#include "printf.h"
#endif 



void timer_i_process(void) {
	
	__disable_irq();
	
	/*
	// get pending requests
	while ( pending messages to i-process ) {
		insert envelope into the timeout queue ;
	}
	while ( first message in queue timeout expired ) {
		msg_t * env = dequeue ( timeout_queue ) ;
		int target_pid = env->destination_pid ;
		// forward msg to destination
		send_message ( target_pid , env ) ;
	}
	*/
	
	
	__enable_irq();
}

void uart_i_process(void) {
	
	__disable_irq();
	
	
	
	
	#ifdef _DEBUG_HOTKEYS
	
	
	
	
	#endif
	
	
	
	
	__enable_irq();
	
}
