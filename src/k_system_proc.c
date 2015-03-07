#include "k_system_proc.h"
#include <LPC17xx.h>
#include "k_ipc.h"
#include "uart_polling.h"
#include "uart.h"
#include "util.h"

/* The process for the keyboard command decoder */
void kcd_proc(void) {
	
	msgbuf* message;
	
	while(1) {
		
		//will block the process at first
		message = receive_message(NULL);
	
		if (message->mtype == DEFAULT) {
			//handle a command
			
			
		} else if (message->mtype == KCD_REG) {
			//register a command
			
			
		}
	
	}
	
}

void crt_proc(void) {
	
	msgbuf *message;
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *) LPC_UART0;
	
	while(1) {
		
		//will block the process at first
		message = receive_message(NULL);
		
		//if message is of a CRT request type, send to uart iproc
		if (1) { //TODO: change condition
			send_message(PID_UART, message);
		}
		
		//release the memory
		release_memory_block(message);
		
	}
	
}

void wall_clock_proc(void) {
	
	msgbuf* message;
	
	//register the WR, WS, and WT commands
	message = request_memory_block();
	message->mtype = KCD_REG;
	copy_string("%WR", message->mtext);
	send_message(PID_KCD, message);
	
	message = request_memory_block();
	message->mtype = KCD_REG;
	copy_string("%WS", message->mtext);
	send_message(PID_KCD, message);
	
	message = request_memory_block();
	message->mtype = KCD_REG;
	copy_string("%WT", message->mtext);
	send_message(PID_KCD, message);
	
	
	while(1) {
		
		message = receive_message(NULL);
		
		//emumerate through all clock commands
		//act depending on the command given
		
		
		
	}
	
}
