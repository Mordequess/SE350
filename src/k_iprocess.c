#include "k_iprocess.h"
#include <LPC17xx.h>
#include "rtx.h"
#include "uart_polling.h"
#include "uart.h"
#include "hot_keys.h"

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
	
	uint8_t IIR_IntId;	    /* Interrupt ID from IIR 		 */
	LPC_UART_TypeDef *pUart = (LPC_UART_TypeDef *)LPC_UART0;
	
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
		
		g_buffer[12] = g_char_in; // nasty hack
		g_send_char = 1;
		
		
	} else if (IIR_IntId & IIR_THRE) {
	/* THRE Interrupt, transmit holding register becomes empty */

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
	      
	} else {  /* not implemented yet */
		
#ifdef DEBUG_0
			uart1_put_string("Should not get here!\n\r");
#endif
		
		return;
	}	
	
}
