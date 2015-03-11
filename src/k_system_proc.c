#include "k_system_proc.h"
#include <LPC17xx.h>
#include "k_ipc.h"
#include "uart_polling.h"
#include "uart.h"
#include "util.h"
#include "rtx.h"
#include "k_iprocess.h"

registered_command g_registered_commands[MAX_COMMANDS];
int g_number_commands_registered = 0;

/* The process for the keyboard command decoder */
void kcd_proc(void) {
	
	int sender_id;
	msgbuf* message;
	msgbuf* message_to_dispatch;
	int i = 0;
	
	while(1) {
		
		//will block the process at first
		message = receive_message(&sender_id);
	
		if (message->mtype == DEFAULT) { 			//handle a command
			
      char keyboard_command_buffer[MAX_COMMAND_LENGTH];
            
			//read the command from the message mdata
      while (i < MAX_COMMAND_LENGTH && message->mtext[i] != '\0' && message->mtext[i] != ' ') {
        keyboard_command_buffer[i] = message->mtext[i];
        i++;
      }
			
			//Loop through the commands to see if this one is registered anywhere
			for (i = 0; i < g_number_commands_registered; i++) {
				if (strings_are_equal(keyboard_command_buffer, g_registered_commands[i].command)) {
					
					message_to_dispatch = request_memory_block();
					message_to_dispatch->mtype = KCD_DISPATCH;
					copy_string(message->mtext, message_to_dispatch->mtext);
					send_message(g_registered_commands[i].process_id, message_to_dispatch);
					break;
					
				}
			}	
			
		} else if (message->mtype == KCD_REG) {

			//Populate the registered_command object
			g_registered_commands[g_number_commands_registered].process_id = sender_id;
			copy_string(message->mtext, g_registered_commands[g_number_commands_registered].command);
			g_number_commands_registered++;
			
		}
					
		release_memory_block(message);
	
	}
	
}

void crt_proc(void) {
	
	int sender_id = -1;
	msgbuf *message;
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *) LPC_UART0;
	
	while(1) {
		
		//will block the process at first
		message = receive_message(&sender_id);
		
		//if message is of a CRT request type, send to uart iproc
		if (message->mtype == CRT_DISP) {
			send_message(PID_UART, message); //will be released at destination
			pUart->IER |= IER_THRE;
		} else {
			release_memory_block(message);
		}
		
	}
	
}

void wall_clock_proc(void) {
	
	int sender_id = -1;
	
	const char RESET = 'R';
	const char TERMINATE = 'T';
	const char SET = 'S';
	
	msgbuf* message;
	msgbuf* delayed_message;
	msgbuf* crt_message;
	
	int current_clock_time = 0;
	int is_clock_running = 0;
	
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
		
		sender_id = -1;
		
		message = receive_message(&sender_id);
		
		if (message->mtype == WALL_CLOCK_TICK) {
			
			if (is_clock_running) {
				
				//update by one second
				current_clock_time++;
				if (current_clock_time >= 24*60*60) {
					current_clock_time = 0;
				}
			
				//Send the new time string hh:mm:ss to CRT
				crt_message = request_memory_block();
				crt_message->mtype = CRT_DISP;
				time_to_hms(current_clock_time, crt_message->mtext);
				send_message(PID_CRT, crt_message);
				
				//send an update message in 1000ms
				delayed_message = request_memory_block();
				delayed_message->mtype = WALL_CLOCK_TICK;
				delayed_message->mtext[0] = '\0'; //wall clock does not care about mtext
				delayed_send(PID_WALL_CLOCK, delayed_message, 1000);
				
			}
			
			release_memory_block(message);
			
		} else {
			
			//The message is one of the 3 commands (start, terminate, reset)
			if (message->mtext[0] == '%' && message->mtext[1] == 'W') {
				
				switch (message->mtext[2]) {
					case RESET:
						is_clock_running = 1;
						current_clock_time = 0;
					
						//send an update message in 1000ms
						delayed_message = request_memory_block();
						delayed_message->mtype = WALL_CLOCK_TICK;
						delayed_message->mtext[0] = '\0';
						delayed_send(PID_WALL_CLOCK, delayed_message, 1000);
					
						release_memory_block(message);
						break;
					
					case TERMINATE:
						is_clock_running = 0;
						current_clock_time = 0;
						release_memory_block(message);
						break;
					
					case SET:
						is_clock_running = 1;
						
						//for %WS hh:mm:ss, skip straight to the date part.
						current_clock_time = time_to_sss(&(message->mtext[4]));
					
						//send display message to crt process
						crt_message = request_memory_block();
						crt_message->mtype = CRT_DISP;
						time_to_hms(current_clock_time, crt_message->mtext);
						send_message(PID_CRT, crt_message);
					
						//send an update message in 1000ms
						delayed_message = request_memory_block();
						delayed_message->mtype = WALL_CLOCK_TICK;
						delayed_message->mtext[0] = '\0';
						delayed_send(PID_WALL_CLOCK, delayed_message, 1000);
					
						//release the original message with the set command.
						release_memory_block(message);
					
						break;
				}
				
			}
			
		}
		
	}
	
}

//helper functions

//parameter is a time in the form (0 = midnight) + (time = #seconds)
void time_to_hms(int sss, char* target) {
	int t;

	t = sss / 3600;
	target[0] = (char)(((int)'0') + t/10);
	target[1] = (char)(((int)'0') + t%10);
	target[2] = ':';

	t = sss / 60;
	target[3] = (char)(((int)'0') + t/10);
	target[4] = (char)(((int)'0') + t%10);
	target[5] = ':';

	t = sss % 60;
	target[6] = (char)(((int)'0') + t/10);
	target[7] = (char)(((int)'0') + t%10);
	
	//newline
	target[8] = '\r';
	target[9] = '\n';
	
	//null terminator
	target[10] = '\0';
}

//parameter is a time in the form (hh:mm:ss)
int time_to_sss(char* hms) {
	
	int t = 0;
	
	assert(str_len(hms) >= 8, "Insufficient length of time string");

	t += (int)((char)hms[0]) * 36000;
	t += (int)((char)hms[1]) * 3600;

	t += (int)((char)hms[3]) * 600;
	t += (int)((char)hms[4]) * 60;

	t += (int)((char)hms[6]) * 10;
	t += (int)((char)hms[7]);

	return t;
}
