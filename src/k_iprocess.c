#include "k_iprocess.h"
#include <LPC17xx.h>
#include "rtx.h"
#include "uart_polling.h"
#include "uart.h"
#include "hot_keys.h"
#include "util.h"
#include "k_ipc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

#ifdef _DEBUG_HOTKEYS
#include "printf.h"
#endif 

uint8_t g_buffer[]= "You Typed a Q\n\r";
uint8_t *gp_buffer = g_buffer;
uint8_t g_send_char = 0;

uint8_t g_char_in;
uint8_t g_char_out;

uint8_t g_input_buffer[BUFFER_SIZE];
uint8_t g_ouptut_buffer[BUFFER_SIZE];
int g_input_buffer_index = 0;
int g_output_buffer_index = 0;

msgbuf* g_msg_uart;

volatile uint32_t g_timer_count = 0; // increment every 1 ms

void timer_i_process(void) {
	
	message* current_message;
		
	/* ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM */
	LPC_TIM0->IR = BIT(0); 
	
	__disable_irq();
	
	g_timer_count++;
  
	//delayed_send put the messages on the queue.
	//iterate through the queue and send expired messages
	current_message = m_peek(PID_TIMER);
	while (current_message != NULL) {
		if (current_message->expiry_time <= get_system_time()) {
			send_message(current_message->destination_id, current_message->message_envelope);
			m_remove_queue_node(PID_TIMER, current_message);
			current_message = m_peek(PID_TIMER);
		} else {
			current_message = current_message->mp_next;
		}
	}
	
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

/*
The message (in the memory block pointed to by the second parameter) 
will be sent to the destination process (process_id) after the expiration
of the delay (timeout, given in millisecond units).
*/
int k_delayed_send(int process_id, void *message_envelope, int delay) {
	
	message *m;
	
	if (delay < 0) {
		return RTX_ERR;
	}
	
	if (process_id < PID_NULL || process_id >= NUM_PROCESSES) {
		return RTX_ERR;
	}
	
	m = message_new(get_procid_of_current_process(), process_id, (msgbuf *)message_envelope, delay);
	m_enqueue(PID_TIMER, m);
	
	return RTX_OK;
}

void uart_i_process(void) {
	
	uint8_t IIR_IntId;	    /* Interrupt ID from IIR 		 */
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
	
	msgbuf* message_to_crt;
	msgbuf* message_to_kcd;
	
	int sender_id;
	
#ifdef DEBUG_0
	uart1_put_string("Entering c_UART0_IRQHandler\n\r");
#endif

	/* Reading IIR automatically acknowledges the interrupt */
	IIR_IntId = (pUart->IIR) >> 1 ; /* skip pending bit in IIR */
	
	if (IIR_IntId & IIR_RDA) { /* Receive Data Available */
		
		/* read UART. Read RBR will clear the interrupt */
		g_char_in = pUart->RBR;
		
#ifdef DEBUG_0
		uart1_put_string("Reading a char = ");
		uart1_put_char(g_char_in);
		uart1_put_string("\n\r");
#endif
		
//hotkey processing. If input_char is not a hotkey, nothing will happen.
#ifdef _DEBUG_HOTKEYS
	process_hot_key(g_char_in);
#endif
		
		//Always send each new character to CRT process
		message_to_crt = request_memory_block();
		message_to_crt->mtype = CRT_DISP;
		message_to_crt->mtext[0] = g_char_in;
		message_to_crt->mtext[1] = '\0';
		send_message(PID_CRT, message_to_crt);
		
		//If we are at a newline, the command is over. Pass to KCD.
		//Otherwise, continue adding to the input buffer
		if (g_char_in == '\r') {
			g_input_buffer[g_input_buffer_index] = '\0';
			
			message_to_kcd = request_memory_block();
			message_to_kcd->mtype = DEFAULT;
			copy_string(g_input_buffer, message_to_kcd->mtext);
			send_message(PID_KCD, message_to_kcd);
			
			g_input_buffer_index = 0;
			
		} else {
			g_input_buffer[g_input_buffer_index] = g_char_in;
			g_input_buffer_index++;
		}
		
	} else if (IIR_IntId & IIR_THRE) {
	/* THRE Interrupt, transmit holding register becomes empty */
		
		if (g_msg_uart == NULL) {
			g_msg_uart = receive_message(&sender_id);
		}
		
		if (g_msg_uart->mtext[g_output_buffer_index] != '\0' ) {
			//character is non-null. Write to THR
			
#ifdef DEBUG_1
	printf("UART i-process: writing %c\n\r", gp_cur_msg->m_data[g_output_buffer_index]);
#endif
            
			pUart->THR = g_msg_uart->mtext[g_output_buffer_index];
            
			g_output_buffer_index++;
            
    } else { //buffer reaches the null terminator
            
      pUart->IER &= (~IER_THRE);
      pUart->THR = '\0';
            
      release_memory_block(g_msg_uart);
            
			g_output_buffer_index = 0;
		}
		
/*
		if (*gp_buffer != '\0' ) {
			g_char_out = *gp_buffer;
			
			
#ifdef DEBUG_0
			printf("Writing a char = %c \n\r", g_char_out);
#endif
			
			pUart->THR = g_char_out;
			gp_buffer++;
			
		} else {
			
#ifdef DEBUG_0
			uart1_put_string("Finish writing. Turning off IER_THRE\n\r");
#endif
			
			pUart->IER ^= IER_THRE; // toggle the IER_THRE bit 
			pUart->THR = '\0';
			g_send_char = 0;
			gp_buffer = g_buffer;		
		}
		*/
	      
	} else {  /* not implemented yet */
		
#ifdef DEBUG_0
			uart1_put_string("Should not get here!\n\r");
#endif
		
		return;
	}	
	
}

/*
Returns the system time in case functions outside this file need it
*/
int get_system_time() {
	return g_timer_count;
}
