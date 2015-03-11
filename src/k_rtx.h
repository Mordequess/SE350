 /** 
 * @file:   k_rtx.h
 * @brief:  kernel deinitiation and data structure header file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */
 
#ifndef K_RTX_H_
#define K_RTX_H_

/*----- Definitions -----*/

#define RTX_ERR -1
#define RTX_OK  0

#define NULL 0
#define NUM_TEST_PROCS 6

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200         /* user proc stack size 512B   */
#else
#define USR_SZ_STACK 0x100         /* user proc stack size 256B  */
#endif /* DEBUG_0 */

#define DEFAULT 0 /* A general purpose message.*/
#define KCD_REG 1 /* A message to register a command with the Keyboard Command Decoder Process */
#define KCD_DISPATCH 2 /* A message to dispatch a registered KCD command to a process */
#define CRT_DISP 3			/* A message intended for the CRT process */
#define WALL_CLOCK_TICK 4 /* A message that updates the wall clock by one second */


//#include "k_ipc.h"
/*----- Types -----*/
typedef unsigned char U8;
typedef unsigned int U32;

/* process states (no need for an EXIT state) */
typedef enum {NEW = 0, READY, RUNNING, BLOCKED_ON_MEMORY, BLOCKED_ON_RECEIVE} PROC_STATE_E;  

//their required message format
typedef struct msgbuf {
	int mtype; /* user defined message type. One of message constants above  */
	char mtext[10]; /* body of the message */
} msgbuf;

//msgbuf wrapper containing the rest of the information we need for message passing
typedef struct message {
	msgbuf* message_envelope;
	int sender_id;
	int destination_id;
	int expiry_time;
	struct message* mp_next;
} message;


/*
  PCB data structure definition.
  You may want to add your own member variables
  in order to finish P1 and the entire project 
*/
typedef struct pcb 
{ 
	U32 m_pid;		/* process id */
	U32 m_priority;
	PROC_STATE_E m_state;   /* state of the process */  
	U32 *mp_sp;		/* stack pointer of the process */
	struct pcb *mp_next;  /* next pcb, not used in this example */  
	message* m_queue; 	/* message queue for this process */
} pcb;

#endif // ! K_RTX_H_
