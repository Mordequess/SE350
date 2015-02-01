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

/* ----- Functions ----- */

void process_init(void);               /* initialize all procs in the system */
pcb *scheduler(void);                  /* pick the pid of the next to run process */
int k_release_process(void);           /* kernel release_process function */

extern U32 *alloc_stack(U32 size_b);   /* allocate stack for a process */
extern void __rte(void);               /* pop exception stack frame */
extern void set_test_procs(void);      /* test process initial set up */

int get_process_priority(int);
int set_process_priority(int, int);
void null_process(void);

pcb *get_pcb_pointer_from_process_id(int);
pcb* get_next_ready_process(void);
pcb* get_next_blocked_process(void);
U32 is_a_more_important_process_ready(pcb* currentProcess);
queue_node* get_queue_node_for_process(pcb* proc);

void block_current_process(void);
U32 unblock_and_switch_to_blocked_process(void);

#endif /* ! K_PROCESS_H_ */
