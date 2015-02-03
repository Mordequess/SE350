#include <LPC17xx.h>
#include "rtx.h"
#include "k_memory.h"
#include "uart_polling.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

int main() {
	
	SystemInit();
	
	uart0_init();
	uart0_put_string("Hello World!\n\r");
	
	#ifdef DEBUG_0
		init_printf(NULL, uart_put_char);
		printf("This is working %d\n\r", 1);
	#endif /* DEBUG_0 */
	
	/* start the RTX and built-in processes */
	k_rtx_init();  
	
	//Should never reach here.
	return RTX_ERR;	
}
