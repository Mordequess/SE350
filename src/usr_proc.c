#include "usr_proc.h"
#include "rtx.h"
#include "uart_polling.h"
#include "util.h"
#include "k_ipc.h"
#include "k_iprocess.h"

#ifdef DEBUG_0
#include "printf.h"
#endif 

#define NUM_BLOCKS 30

const int RUN_LENGTH = 27;
int expected_proc_order[] = 
{
 1, 2, 3, 3, 1, 1, 2, 2, 2, 3, 1, 3, 3, 2,
	4, 4, 5, 4, 5, 4, 6, 4, 5, 4, 6, 5, 5
 
};
int actual_proc_order[RUN_LENGTH];
int current_index = 0;

const int TOTAL_TESTS = 8;
int total_passed_tests = 0;
int total_failed_tests = 0;

/* Adds the indicated process to the runtime history. */
void add_to_order(int proc_id) {
	actual_proc_order[current_index] = proc_id;
	current_index++;
}

/* Returns 1 if expected sequence order has occurred. */
int check_order() {
	int i;
	for (i = 0; i < current_index; i++) {
		if (actual_proc_order[i] != expected_proc_order[i]) {
			return 0;
		}
	}
	return 1;
}

/*
Logs an OK test if test_flag is true and check_order() gives expected order
Otherwise, the test is logged as a failure
*/
void submit_test(unsigned char* test_number, int test_flag) {
	uart0_put_string("G028_test: test ");
	uart0_put_string(test_number);
	uart0_put_string(" ");
	if (test_flag && check_order()) {
		uart0_put_string("OK\n\r");
		total_passed_tests++;
	} else {
		uart0_put_string("FAIL\n\r");
		total_failed_tests++;
	}
}

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_stack_size=0x260;
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


void proc1(void) {
	int i;
	int status[6];
	int testPassed = 1;
	void* mem_ptr;
	
	uart0_put_string("G028_test: START\n\r");
	//get invalid pids
	status[0] = get_process_priority(-1);
	status[1] = get_process_priority(19);

	//try setting null process priority
	status[2] = set_process_priority(0, 1);

	//try setting KCD (system) priority
	status[3] = set_process_priority(PID_KCD, 2);

	//try setting valid process's priority to null process priority
	status[4] = set_process_priority(1, NULL_PRIORITY);

	//try setting valid process's priority to an invalid priority
	status[5] = set_process_priority(1, 88);


	for (i = 0; i < 6; i++) {
		if (status[i] != RTX_ERR) {
			testPassed = 0;
		}
	}
	
	add_to_order(1);
	submit_test("1", testPassed);
	
	//should take us to proc 2
	release_processor();
		
	//Proc 1 is back.
	add_to_order(1);
	testPassed = 1;
	
	//set_process_priority(4, LOWEST);
	//set_process_priority(4, LOW);
	
	//neither call should have pre-empted. TODO: they are
	add_to_order(1);
	//submit_test("5", testPassed);
	
	//request memory so we get blocked and go to proc2.
	mem_ptr = request_memory_block();
	
	//back in P1.
	add_to_order(1);
	if (mem_ptr == NULL) testPassed = 0;
	
	//submit_test("7", testPassed);
	
	release_memory_block(mem_ptr);
	
	//P3 will take over.
	set_process_priority(1, LOWEST);
	
	while(1) {
		release_processor();
	}
	
}

//verifies get_process_priority() and uses set_process_priority.
void proc2(void){
	int i = 0;
	int testPassed = 1;
	
	int expected_priorities[] = {NULL_PRIORITY, MEDIUM, MEDIUM, LOW, LOW, LOW, LOW};
	int actual_priorities[7];

	add_to_order(2);

	for (i = 0; i < 7; i++) {
		actual_priorities[i] = get_process_priority(i);
		if (actual_priorities[i] != expected_priorities[i]) {
			testPassed = 0;
		}
	}
	
	//submit_test("2", testPassed);
	
	//should pre-empt to proc3.
	set_process_priority(3, HIGH);
	
	//proc2 is back.
	testPassed = 1;
	add_to_order(2);
	
	//expecting different priorities now (only for P3)
	expected_priorities[3] = LOWEST;
	for (i = 0; i < 7; i++) {
		actual_priorities[i] = get_process_priority(i);
		if (actual_priorities[i] != expected_priorities[i]) {
			testPassed = 0;
		}
	}
	add_to_order(2);
	//submit_test("6", testPassed);
	
	//should not pre-empt because 1 is blocked on memory
	set_process_priority(1, HIGH);
	add_to_order(2);
	
	//pre-empt.
	set_process_priority(3, HIGH);
	
	//back in P2.
	add_to_order(2);
	
	//P4 will take over.
	set_process_priority(2, LOWEST);
	
	while(1) {
		release_processor();
	}
	
}

void proc3(void) {
	int i = 0;
	int status = 0;
	void* mem_ptr[100];
	int testPassed = 1;
	
	add_to_order(3);
	
	// Hog all the memory blocks
	while (hasFreeSpace()) {
		mem_ptr[i] = request_memory_block();
		if (mem_ptr[i] == NULL) {
			testPassed = 0;
		}
		i++;
	}
	
	//Undo the last increment
	i--;
	
	//test an invalid release
	status = release_memory_block((void*)1);
	if (status != RTX_ERR) {
		testPassed = 0;
	}
	
	//submit_test("3", testPassed);
	testPassed = 1;
	
	release_processor(); //should keep us in proc3 since this is the only HIGH process
	
	add_to_order(3);
	//submit_test("4", testPassed); //check_order() alone will decide whether this passed
	
	//set self to LOWEST so something else can run (it will be proc1, which is MEDIUM)
	set_process_priority(3, LOWEST);
	
	//proc 3 is back.
	testPassed = 1;
	add_to_order(3);
	
	//by releasing a block, P1 will become unblocked.
	//P1 and P3 are HIGH, P2 is MEDIUM. P1 takes over.
	release_memory_block(mem_ptr[i]);
	i--;
	
	//back in P3.
	add_to_order(3);
	testPassed = 1;
	
	for (; i > 0; i--) {
		status = release_memory_block(mem_ptr[i]);
		if (status != RTX_OK) testPassed = 0;
	}
	
	status = release_memory_block(mem_ptr[0]);
	if (status != RTX_OK) testPassed = 0;
	
	//releasing should never have pre-empted.
	add_to_order(3);
	submit_test("2", testPassed);
	
	//end of process. Switch to P2.
	set_process_priority(3, LOWEST);
	
	while(1) {
		release_processor();
	}
}


void proc4(void){
	
	msgbuf* message;
	int sender_id = NULL; //storage for message sender
	int testPassed = 1;

	//P1, P2, and P3 should be finished by the time P4 starts
	add_to_order(4);
	
	//should not pre-empt at any point. Should remain inside P4.
	set_process_priority(4, HIGH);
	set_process_priority(5, MEDIUM);
	set_process_priority(6, MEDIUM);
	
	add_to_order(4);
	submit_test("3", testPassed);

	//waiting for message from 5. Block and go to P5.
	message = receive_message(&sender_id);

	//message received
	add_to_order(4);
	testPassed = 1;
	if (message->mtype != DEFAULT || (message->mtext[0] != 'Q')) {
		testPassed = 0;
	}
	release_memory_block(message);
	
	submit_test("4", testPassed);

	//jump to 5
	set_process_priority(5, HIGH);

	//5 will get blocked on recieve, jump to 6
	add_to_order(4);
	
	//reserve a memory block for a message, then send in the gobbler
	message = request_memory_block();
	set_process_priority(6, HIGH);

	//6 will get blocked on memory
	add_to_order(4);

	message->mtype = DEFAULT;
	message->mtext[0] = 'R';
	send_message(5, message); //should pre-empt on send?

	//at this point 6 is being blocked, there is a memory block available to it
	add_to_order(4);
	submit_test("5", testPassed);
	set_process_priority(4, LOWEST);

	while(1) {
		release_processor();
	}
}

void proc5(void){
	void* mem_ptr;
	int testPassed = 1;
	int sender_id = NULL;
	
	msgbuf* message = request_memory_block();
	add_to_order(5);
	message->mtype = DEFAULT;
	message->mtext[0] = 'Q';
	send_message(4, message); //should pre-empt to 4.

	//back at 5. time to test blocked on memory & blocked on receive
	add_to_order(5);
	mem_ptr = request_memory_block(); //request one memblock to block p6
	message = receive_message(&sender_id); //get blocked on receive, go to p4
	
	//message received
	add_to_order(5);
	if (sender_id != 4 || message->mtext[0] != 'R') {
		testPassed = 0;
	}
	else {
		release_memory_block(mem_ptr);  //will jump back to p4. P6 will take this later
	}
	release_memory_block(message);
	
	add_to_order(5);
	submit_test("7", testPassed);

	//delayed send to self
	message = request_memory_block();
	message->mtype = DEFAULT;
	message->mtext[0] = 'S';
	delayed_send(5, message, 5);
	message = receive_message(&sender_id);

	add_to_order(5);
	if (sender_id != 5 || message->mtext[0] != 'S') {
		testPassed = 0;
	}
	release_memory_block(message);
	
	//submit test, should go on to p7 (or end)
	submit_test("8", testPassed);
	
	uart0_put_string("G028_test: ");
	uart0_put_char('0' + total_passed_tests);
	uart0_put_string("/");
	uart0_put_char('0' + TOTAL_TESTS);
	uart0_put_string(" tests OK \n\r");
	
	uart0_put_string("G028_test: ");
	uart0_put_char('0' + total_failed_tests);
	uart0_put_string("/");
	uart0_put_char('0' + TOTAL_TESTS);
	uart0_put_string(" tests FAIL \n\r");
	
	uart0_put_string("G028_test: END\n\r");

	set_process_priority(5, LOWEST);
	
	while(1) {
		release_processor();
	}
}

void proc6(void){
	int i;
	void* mem[100];
	int testPassed = 1;

	add_to_order(6);
	//will get blocked on memory, go to 4
	for (i = 0; i < NUM_BLOCKS; ++i) {
		mem[i] = request_memory_block();
	}
	
	add_to_order(6);

	for (i = 0; i < NUM_BLOCKS; ++i) {
		release_memory_block(mem[i]);
	}

	submit_test("6", testPassed);
	set_process_priority(6, LOWEST);

	while(1) {
		release_processor();
	}
}
