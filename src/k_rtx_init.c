/** 
 * @file:   k_rtx_init.c
 * @brief:  Kernel initialization C file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_rtx_init.h"
#include "rtx.h"
#include "uart_polling.h"
#include "timer.h"
#include "uart.h"


void k_rtx_init(void)
{
	__disable_irq();
	uart_irq_init(0);   // uart0, interrupt-driven 
	
	timer_init(0);			//timer0 initialization
	timer_init(1);			//timer1 for testing
	
	uart1_init();       // uart1, polling
	memory_init();
	process_init();
	__enable_irq();
	
	//uart0_put_string("Type 'S' in COM0 terminal to switch between proc1 and proc2 or wait for them to switch between themselves\n\r");
	//uart0_put_string("An input other than 'S' in COM0 terminal will be have no effect.\n\r"); 
	
	/* start the first process */
	release_processor();
}
