#include <LPC17xx.h>
#include "uart_polling.h"
#include "rtx.h"

int main() {
	SystemInit();
	uart0_init();
	uart0_put_string("Hello World!\n\r");
	
	/* start the RTX and built-in processes */
	rtx_init();  
	
	return RTX_ERR;
}
