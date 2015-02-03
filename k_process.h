/**
 * @file:   k_process.h
 * @brief:  process management hearder file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: Assuming there are only two user processes in the system
 */

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
#define STACK_SIZE 0x100               /* stack size hardcoded as 0x100 in usr_proc */

/* -------- QUEUES FOR PROCESSES -------- */

void enqueue(pcb* queue, pcb* element);
pcb* dequeue(pcb* queue);
void remove_queue_node(pcb* queue, pcb* element);
U32 is_empty(pcb* queue);

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

void block_current_process(void);
int unblock_and_switch_to_blocked_process(void);

#endif /* ! K_PROCESS_H_ */
