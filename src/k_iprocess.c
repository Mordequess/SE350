#include "k_iprocess.h"
#include <LPC17xx.h>
#include "rtx.h"
#include "uart_polling.h"
#include "uart.h"
#include "hot_keys.h"
#include "util.h"
#include "k_ipc.h"
#include "k_process.h"
#include "k_memory.h"

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

U32 g_timer_flag = 0;
U32 g_uart_flag = 0;

/* From LEARN notes
save the context of the current_process ;
switch the current_process with timer_i_process ;
load the timer_i_process context ;
call the timer_i_process C function ;
invoke the scheduler to pick next to run process ;
restore the context of the newly picked process ;
*/
__asm void TIMER0_IRQHandler(void)
{
  PRESERVE8
  IMPORT timer_i_process
  IMPORT k_release_processor
  PUSH {R4-R11, lr}
  BL timer_i_process
  LDR R4, =__cpp(&g_timer_flag);
  LDR R4, [R4]
  MOV R5, #0
  CMP R4, R5
  BEQ TIMER_RESTORE
  BL k_release_processor
TIMER_RESTORE
  POP {R4-R11, pc}
}

void timer_i_process(void) {
	
	message* current_message;
	message* temp;
		
	/* ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM */
	LPC_TIM0->IR = BIT(0); 
	
	__disable_irq();
	
	g_timer_count++;
	
	g_timer_flag = 0;
  
	//delayed_send put the messages on the queue.
	//iterate through the queue and send expired messages
	current_message = m_peek(PID_TIMER);
	while (current_message != NULL) {
		
		if (current_message->expiry_time <= get_system_time()) {
			temp = current_message;
			m_remove_queue_node(PID_TIMER, current_message);
			
			k_delayed_send_message(temp);
			
			current_message = m_peek(PID_TIMER);
			
			//if priority of receiving process is greater, pre-empt
			if (k_get_process_priority(temp->destination_id) <= k_get_process_priority(get_procid_of_current_process())) {
				g_timer_flag = 1;
			}
			
		} else {
			current_message = current_message->mp_next;
		}
	}
	
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

//CPSID and CPSIE disable and enable interrupts
//iprocess will not be interrupted.
__asm void UART0_IRQHandler(void)
{
  PRESERVE8
  IMPORT uart_i_process
  IMPORT k_release_processor
  PUSH {R4-R11, lr}
  BL uart_i_process
  LDR R4, =__cpp(&g_uart_flag);
  LDR R4, [R4]
  MOV R5, #0
  CMP R4, R5
  BEQ UART_RESTORE
  BL k_release_processor
UART_RESTORE
  POP {r4-r11, pc}
}

void uart_i_process(void) {
	
	uint8_t IIR_IntId;	    /* Interrupt ID from IIR 		 */
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
	
	msgbuf* message_to_crt;
	msgbuf* message_to_kcd;
	
	int sender_id;
	
	g_uart_flag = 0;
	
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
		
		if (hasFreeSpace()) {
			//Always send each new character to CRT process
			message_to_crt = k_request_memory_block();
			message_to_crt->mtype = CRT_DISP;
			message_to_crt->mtext[0] = g_char_in;
			message_to_crt->mtext[1] = '\0';
			k_send_message(PID_CRT, message_to_crt);
		
			//we will want to pre-empt to crt
			g_uart_flag = 1;
		}
		
		
		//If we are at a newline, the command is over. Pass to KCD.
		//Otherwise, continue adding to the input buffer
		if (g_char_in == '\r') {
			g_input_buffer[g_input_buffer_index] = '\0';
			
			if (hasFreeSpace()) {
				message_to_kcd = k_request_memory_block();
				message_to_kcd->mtype = DEFAULT;
				copy_string(g_input_buffer, message_to_kcd->mtext);
				k_send_message(PID_KCD, message_to_kcd);
			
				g_input_buffer_index = 0;
			
				//pre-empt to KCD proc
				g_uart_flag = 1;
				
			}
			
		} else {
			//do not put in input buffer if it's a hotkey
			if (g_char_in != DEBUG_HOTKEY_1 && g_char_in != DEBUG_HOTKEY_2 && g_char_in != DEBUG_HOTKEY_3) {
				g_input_buffer[g_input_buffer_index] = g_char_in;
				g_input_buffer_index++;
			}
		}
		
	} else if (IIR_IntId & IIR_THRE) {
	/* THRE Interrupt, transmit holding register becomes empty */
		
		g_msg_uart = (receive_message_non_blocking(PID_UART))->message_envelope;
		
		if (g_msg_uart != NULL) {
		
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
            
				k_release_memory_block(g_msg_uart);
            
				g_output_buffer_index = 0;
			}
		
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
