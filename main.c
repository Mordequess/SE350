#include <LPC17xx.h>
#include "uart_polling.h"
#include "memory.h"

int main() {
	void* test1;
	void* test2;
	
	SystemInit();
	uart0_init();
	
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
}
