#include "k_system_proc.h"
#include <LPC17xx.h>
#include "k_ipc.h"
#include "uart_polling.h"
#include "uart.h"
#include "util.h"

registered_command g_registered_commands[MAX_NUM_COMMANDS];
int g_number_commands_registered = 0;

/* The process for the keyboard command decoder */
void kcd_proc(void) {
	
	msgbuf* message;
	msgbuf* message_to_dispatch;
	int i = 0;
	
	while(1) {
		
		//will block the process at first
		message = receive_message(NULL);
	
		if (message->mtype == DEFAULT) {
			//handle a command
			
      char keyboard_command_buffer[MAX_COMMAND_LENGTH];
            
			//read the command from the message mdata
      while (i < MAX_COMMAND_LENGTH && message->mdata[i] != '\0' && message->mdata[i] != ' ') {
        keyboard_command_buffer[i] = message->mdata[i];
        i++;
      }
			
			//Loop through the commands to see if this one is registered anywhere
			for (i = 0; i < g_number_commands_registered; i++) {
				if () {
					
					message_to_dispatch = request_memory_block();
					message_to_dispatch->mtype = KCD_DISPATCH;
					copy_string(message->mdata, message_to_dispatch->mdata);
					
					//TODO: send message. Need PID to send it to.
					
					break;
					
				}
			}
			
			
		} else if (message->mtype == KCD_REG) {

			
				/*TODO: register a command. Use sender pid and mtext to fill the registered_command object*/
			
			
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
		if (message->mtype == CRT_DISP) {
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
		
		int sender = PID_WALL_CLOCK;
		
		message = receive_message(&sender);
		
		//emumerate through all clock commands
		//act depending on the command given
		
		
		
	}
	
}
