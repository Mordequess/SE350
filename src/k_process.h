#ifndef K_PROCESS_H_
#define K_PROCESS_H_

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_rtx.h"
#include "k_memory.h"
#include "rtx.h"


/* ----- Definitions ----- */

#define INITIAL_xPSR 0x01000000        /* user process initial xPSR value */
#define STACK_SIZE 0x200               /* stack size hardcoded as 0x100 in usr_proc */ //TODO: make it not

/* -------- QUEUES FOR PROCESSES -------- */

void enqueue(pcb** targetQueue, pcb* element);
pcb* dequeue(pcb** targetQueue);
void remove_queue_node(pcb** targetQueue, pcb* element);
U32 is_empty(pcb* targetQueue);

/* ----- Functions ----- */

extern void __rte(void);

void process_init(void);               /* initialize all procs in the system */
pcb *scheduler(void);                  /* pick the pid of the next to run process */
int k_release_process(void);           /* kernel release_process function */

extern U32 *alloc_stack(U32 size_b);   /* allocate stack for a process */
extern void __rte(void);               /* pop exception stack frame */
extern void set_test_procs(void);      /* test process initial set up */

int k_get_process_priority(int);
int k_set_process_priority(int, int);
void null_process(void);

pcb *get_pcb_pointer_from_process_id(int);
U32 is_a_more_important_process_ready(pcb* currentProcess);

void block_current_process_on_memory(void);
void block_current_process_on_receive(void);
int unblock_and_switch_to_blocked_on_memory_process(void);
int unblock_and_switch_to_blocked_on_receive_process(pcb* );
int get_procid_of_current_process(void);

pcb* get_ready_queue(void);
pcb* get_blocked_on_memory_queue(void);
pcb* get_blocked_on_receive_queue(void);

#endif /* ! K_PROCESS_H_ */
