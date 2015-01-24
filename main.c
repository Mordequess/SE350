#include <LPC17xx.h>
#include "uart_polling.h"
#include "rtx.h"
#include "k_memory.h"

int main() {
	
	SystemInit();
	uart0_init();
	uart0_put_string("Hello World!\n\r");
	
	/* start the RTX and built-in processes */
	rtx_init();  
	
	return RTX_ERR;

	//OLD TESTING CODE.
	//AT THE END OF RTX_INIT(), THE PROCESSOR IS ASSIGNED TO PROCESSES
	//TESTS LIKE THIS CAN NO LONGER BE DONE FROM HERE.
	/*
	void* test1;
	void* test2;
	memory_init();
	test1 = 0;
	test2 = 0;
	
	test1 = request_memory_block();
	if (test1 == 0) uart0_put_string("Was 0\n\r");
	else uart0_put_string("Not 0!\n\r");
	
	test2 = request_memory_block();
	if (test2 >= test1) uart0_put_string("greater or equal\n\r");
	else uart0_put_string("Not greater!\n\r");
	
	return 0;
	*/
	
}
