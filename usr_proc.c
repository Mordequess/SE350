#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "util.h"

#ifdef DEBUG_0
#include "printf.h"
#endif 

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_stack_size=0x100;
	}
  
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[0].m_priority   = MEDIUM;
	
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[1].m_priority   = MEDIUM;
	
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[2].m_priority   = LOW;
	
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[3].m_priority   = LOW;
	
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[4].m_priority   = LOW;
	
	g_test_procs[5].mpf_start_pc = &proc6;
	g_test_procs[5].m_priority   = LOW;
	
}

/*
EXPECTED OUTPUT

12345																					//(proc1)
NULLPROCESS MEDIUM MEDIUM LOW LOW LOW LOW 		//(proc2)
60 memory blocks taken by proc3								//(proc3)
In proc3																			//(proc3)
6789																					//(proc1)
proc1 has reached the end.										//(proc1)
NULLPROCESS MEDIUM MEDIUM LOWEST LOW LOW LOW	//(proc2)
60 memory blocks taken by proc4								//(proc4)	
proc2 has reached the end.										//(proc2)
proc3 about to release 1 block								//(proc3)
proc4 is free again														//(proc4)
proc4 has reached the end.										//(proc4)
In proc5. All checks passed.									//(proc5)
proc5 has reached the end.										//(proc5)
50 memory blocks taken by proc6								//(proc6)
proc6 has reached the end.										//(proc6)
proc3 has reached the end.										//(proc3)

*/

//prints the number 1-9 to the screen. Releases at 5.
void proc1(void)
{
	int i = 1;
	
	for (i = 1; i < 10; i++) {
		uart0_put_char('1' + i);
		
		if (i == 5) {
			uart0_put_string("\n\r");
			release_processor(); //will take us to proc2
		}
	}
	
	uart0_put_string("\n\r");
	uart0_put_string("proc1 has reached the end.\n\r");
	
	//on the first call, this will go to proc2
	while(1) {
		release_processor();
	}
	
}

//verifies get_process_priority() and uses set_process_priority.
void proc2(void)
{
	int i = 0;
	int priority;
	
	//read and output all priorities.
	for (i = 0; i <= 6; i++) {
		priority = get_process_priority(i);
		switch(priority) {
			case '0':
				uart0_put_string("HIGH ");
				break;
			case '1':
				uart0_put_string("MEDIUM ");
				break;
			case '2':
				uart0_put_string("LOW ");
				break;
			case '3':
				uart0_put_string("LOWEST ");
				break;
			case '4':
				uart0_put_string("NULLPROCESS ");
		}
			
	}
	
	uart0_put_string("\n\r");
	
	//set process 3 to HIGH. This should pre-empt.
	set_process_priority(3, HIGH);
	
	//read and output all priorities again.
	for (i = 0; i <= 6; i++) {
		priority = get_process_priority(i);
		switch(priority) {
			case '0':
				uart0_put_string("HIGH ");
				break;
			case '1':
				uart0_put_string("MEDIUM ");
				break;
			case '2':
				uart0_put_string("LOW ");
				break;
			case '3':
				uart0_put_string("LOWEST ");
				break;
			case '4':
				uart0_put_string("NULLPROCESS ");
		}
		
	}
	
	set_process_priority(4, HIGH); //proc4 will take over.
	
	uart0_put_string("proc2 has reached the end.\n\r");
	while(1) {
		release_processor();
	}
	
}

void proc3(void)
{
	int i = 0;
	int status = 0;
	void *mem_ptr[60];
	
	for (i = 0; i < 60; i++) {
		mem_ptr[i] = request_memory_block();
		assert(mem_ptr[i] != NULL, "request_memory_block returned NULL");
	}
	uart0_put_string("60 memory blocks taken by proc3\n\r");
	
	//test an invalid release
	status = release_memory_block((void*)1);
	assert(status == RTX_ERR, "Invalid address was released");
	
	release_processor(); //should keep us in proc3 since this is the only HIGH process
	
	uart0_put_string("In proc3\n\r");
	
	//set self to LOWEST so something else can run (it will be proc1, which is MEDIUM)
	set_process_priority(3, LOWEST);
	
	uart0_put_string("Proc3 about to release 1 block\n\r");
	status = release_memory_block(mem_ptr[0]); //should unblock proc4
	
	for (i = 1; i < 60; i++) {
		status = release_memory_block(mem_ptr[i]);
		assert(status == RTX_OK, "Release was not OK");
	}
	
	uart0_put_string("proc3 has reached the end.\n\r");
	while(1) {
		release_processor();
	}
}


void proc4(void)
{
	int i = 0;
	int status = 0;
	void *mem_ptr[40];
	void *surplus_ptr;
	
	//request 40 memory blocks, putting us at the limit.
	for (i = 0; i < 40; i++) {
		mem_ptr[i] = request_memory_block();
		assert(mem_ptr[i] != NULL, "request_memory_block returned NULL");
	}
	uart0_put_string("40 memory blocks taken by proc4\n\r");
	
	//set 3, 5, and 6 to MEDIUM priority and puts them on MEDIUM queue
	//should not pre-empt since this process is currently HIGH.
	set_process_priority(3, MEDIUM);
	set_process_priority(5, MEDIUM);
	set_process_priority(6, MEDIUM);
	
	//request a mem block. Should block this process.
	surplus_ptr = request_memory_block();
	
	//if we're here again, it means we've been unblocked.
	uart0_put_string("proc4 is free again\n\r");
	
	//release everything. Should not pre-empt.
	release_memory_block(surplus_ptr);
	for (i = 0; i < 40; i++) {
		status = release_memory_block(mem_ptr[i]);
		assert(status == RTX_OK, "Memory release was not OK");
	}
	
	//setting to medium. Should not pre-empt.
	set_process_priority(4, MEDIUM);
	
	uart0_put_string("proc4 has reached the end.\n\r");
	
	//on first call, will take us to proc5
	while(1) {
		release_processor();
	}
}

void proc5(void)
{
	int i;
	int status;
	
	//check the priority getters and setters in fringe cases.
	status = get_process_priority(-1);
	assert(status == RTX_ERR, "Invalid process priority get");
	status = get_process_priority(15);
	assert(status == RTX_ERR, "Invalid process priority get");
	status = set_process_priority(0, 1);
	assert(status == RTX_ERR, "Set the null process's priority");
	status = set_process_priority(1, 88);
	assert(status == RTX_ERR, "Set an invalid process priority");
	
	uart0_put_string("In proc 5. All checks passed.\n\r");
	
	uart0_put_string("proc5 has reached the end.\n\r");
	
	//should call proc6 on its first time
	while(1) {
		release_processor();
	}
}

void proc6(void)
{
	void *mem_ptr[50];
	int status;
	int i = 0;
	
	//Too many requests. Will block and go to 3.
	for (i = 0; i < 50; i++) {
		mem_ptr[i] = request_memory_block();
		assert(mem_ptr[i] != NULL, "Memory request not fulfilled");
	}
	
	uart0_put_string("50 memory blocks taken by proc6\n\r");
	
	for (i = 0; i < 50; i++) {
		status = release_memory_block(mem_ptr[i]);
		assert(status == RTX_OK, "Memory release not OK");
	}
	
	uart0_put_string("proc6 has reached the end.\n\r");
	while(1) {
		release_processor();
	}
}
