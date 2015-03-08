#include "usr_proc.h"
#include "rtx.h"
#include "uart_polling.h"
#include "util.h"
#include "k_ipc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif 

const int RUN_LENGTH = 18;
int expected_proc_order[] = 
{1, 2, 3, 3, 1, 1, 2, 2, 2, 3, 1, 3, 3, 2,
4, 4, 5, 4
};
int actual_proc_order[RUN_LENGTH];
int current_index = 0;

const int TOTAL_TESTS = 10;
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
	
	set_process_priority(4, LOWEST);
	set_process_priority(4, LOW);
	
	//neither call should have pre-empted.
	add_to_order(1);
	submit_test("5", testPassed);
	
	//request memory so we get blocked and go to proc2.
	mem_ptr = request_memory_block();
	
	//back in P1.
	add_to_order(1);
	if (mem_ptr == NULL) testPassed = 0;
	
	submit_test("7", testPassed);
	
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
	
	submit_test("2", testPassed);
	
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
	submit_test("6", testPassed);
	
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
	
	// Hog all the 100 memory blocks
	for (i = 0; i < 100; i++) {
		mem_ptr[i] = request_memory_block();
		if (mem_ptr[i] == NULL) {
			testPassed = 0;
		}
	}
	
	//test an invalid release
	status = release_memory_block((void*)1);
	if (status != RTX_ERR) {
		testPassed = 0;
	}
	
	submit_test("3", testPassed);
	testPassed = 1;
	
	release_processor(); //should keep us in proc3 since this is the only HIGH process
	
	add_to_order(3);
	submit_test("4", 1); //check_order() alone will decide whether this passed
	
	//set self to LOWEST so something else can run (it will be proc1, which is MEDIUM)
	set_process_priority(3, LOWEST);
	
	//proc 3 is back.
	testPassed = 1;
	add_to_order(3);
	
	//by releasing a block, P1 will become unblocked.
	//P1 and P3 are HIGH, P2 is MEDIUM. P1 takes over.
	release_memory_block(mem_ptr[0]);
	
	//back in P3.
	add_to_order(3);
	testPassed = 1;
	
	for (i = 1; i < 100; i++) {
		status = release_memory_block(mem_ptr[i]);
		if (status != RTX_OK) testPassed = 0;
	}
	
	//releasing should never have pre-empted.
	add_to_order(3);
	submit_test("8", testPassed);
	
	//end of process. Switch to P2.
	set_process_priority(3, LOWEST);
	
	while(1) {
		release_processor();
	}
}


void proc4(void){
	
	msgbuf* message;
	int sender_id = 5;
	int testPassed = 1;

	//P1, P2, and P3 should be finished by the time P4 starts
	add_to_order(4);
	
	//should not pre-empt at any point. Should remain inside P4.
	set_process_priority(4, HIGH);
	set_process_priority(5, MEDIUM);
	set_process_priority(6, MEDIUM);
	
	add_to_order(4);
	submit_test("9", testPassed);

	//waiting for message from 5. Block and go to P5.
	message = receive_message(&sender_id);

	//No longer blocked.
	add_to_order(4);
	
	testPassed = 1;
	if (message->mtype != DEFAULT || (message->mtext[0] != 'Q')) {
		testPassed = 0;
	}
	
	submit_test("10", testPassed);

	

	while(1) {
		release_processor();
	}
}

/* ~~~~~~~ ORIGINAL DEFINITION OF PROC5 ~~~~~~~ */
// void proc5(void){
// 	int testPassed = 1;
// 	
// 	msgbuf* message = request_memory_block();
// 	message->mtype = DEFAULT;
// 	message->mtext[0] = 'Q';
// 	
// 	add_to_order(5);
// 	
// 	//should pre-empt to 4.
// 	send_message(4, message);

// 	while(1) {
// 		release_processor();
// 	}
// }

/* ~~~~~~~ KELLY TESTING DEF OF PROC6 ~~~~~~~~ */
void proc5(void) {
	void *p_msg;
	msgbuf message;
	int* sender;
	// i want to receive a message from proc 6. once i do, i want to uart1putstring the message
	// and then... idk set my own prio low orsomething
	uart0_put_string("kellyPROC5: START\n\r");
	
	p_msg = receive_message(sender); // get a special message!! (we want it to be from 6)
	message = *((msgbuf *)p_msg); // ?/ doesthis makesense
	uart0_put_string(message.mtext);
	
	while(1) {
		uart0_put_string("5 is done\n\r");
		release_processor();
	}
		
}

/* ~~~~~~~ ORIGINAL DEFINITION OF PROC6 ~~~~~~~ */
// void proc6(void){
// 	int testPassed = 1;
// 	
// 	while(1) {
// 		release_processor();
// 	}
// }

/* ~~~~~~~ KELLY TESTING DEF OF PROC6 ~~~~~~~~ */
void proc6(void) {
	void *my_blk;
	msgbuf secret_message = { DEFAULT, 'Q'};//msgbuf contains a type anda character
	// i want to request a block, i want to put a msgbuf in it, i want to send it to proc5 (and then set proc5 to higher prio so it runs)
	uart0_put_string("kellyPROC6: START\n\r");
	
	my_blk = request_memory_block();
	//secret_message = { 0, 6, "you are beautiful in every single way\n\r                       " }; // maybe send_message should really take care of this.
	//secret_message.msource = 6; // now that i'm writing this, it seems dumb. can you send ransom messages?
	//secret_message.mtext = "you are beautiful in every single way\n\r                       "; //41 + 23 spaces????
	my_blk = &secret_message; //uh... i get what i want to do...but...
	send_message(5, my_blk);
	
	set_process_priority(6, LOW); // set me to low
	
	while(1) {
		uart0_put_string("6 is done\n\r");
		release_processor();
	}
}
