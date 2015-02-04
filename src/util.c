#include "util.h"
#include "uart_polling.h"

void assert(int expression, unsigned char * message) {
	
	if (expression == 0) {
		uart0_put_string(message);
		uart0_put_string("\n\r");
		uart0_put_string("ASSERTION FAILED. PROGRAM WILL NOW FREEZE\n\r\n\r");
		while (1) {} //freeze
	}
	
	//if expression is true, do nothing.
	
}
