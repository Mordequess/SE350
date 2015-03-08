#include "util.h"
#include "uart_polling.h"

void assert(int expression, unsigned char * message) {
	
	if (expression == 0) {
		uart0_put_string(message);
		uart0_put_string("\n\r");
		uart0_put_string("ASSERTION FAILED. PROGRAM WILL NOW FREEZE\n\r\n\r");
		while (1) {} //freeze
	}
	
	//if expression is true, do nothing. Just exit.
	
}

void copy_string(char source[], char destination[]) {
	int i = 0;
    
	while (1) {
		destination[i] = source[i];
        
		if (source[i] == '\0') {
			break;
		}
        
		i++;
  }
}

int strings_are_equal(char s1[], char s2[]) {
	
	int i = 0;
	while (s1[i] == s2[i]) {
		if (s1[i] == '\0' || s2[i] == '\0') {
			break;
		}
		i++;
	}
	
	//if strings are equal, we would not have broken until the end
	return (s1[i] == '\0' && s2[i] == '\0');
}
