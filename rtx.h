/* @brief: rtx.h User API prototype, this is only an example
 * @author: Yiqing Huang
 * @date: 2014/01/17
 */
#ifndef RTX_H_
#define RTX_H_

/* ----- Definitions ----- */
#define RTX_ERR -1
#define NULL 0
#define NUM_TEST_PROCS 6

/* Process Priority. The bigger the number is, the lower the priority is*/
#define HIGH    0
#define MEDIUM  1
#define LOW     2
#define LOWEST  3

#define NUM_PRIORITIES 5

/*Process IDs*/
#define PID_NULL 0
#define PID_P1   1
#define PID_P2   2
#define PID_P3   3
#define PID_P4   4
#define PID_P5   5
#define PID_P6   6

/* ----- Types ----- */
typedef unsigned int U32;

/* initialization table item */

typedef struct proc_init
{	
	int m_pid;	        
	int m_priority;         
	int m_stack_size;       
	void (*mpf_start_pc) ();    
} PROC_INIT;


/* -------- QUEUES FOR PROCESSES -------- */

typedef struct queue_node {
	void* contents;
	struct queue_node *next;
} queue_node;

typedef struct queue {
	queue_node* head;
	queue_node* tail;
} queue;

void enqueue(queue*, queue_node*);
queue_node* dequeue(queue*);
U32 is_empty(queue*);

/* ----- RTX User API ----- */
#define __SVC_0  __svc_indirect(0)

extern void k_rtx_init(void);
#define rtx_init() _rtx_init((U32)k_rtx_init)
extern void __SVC_0 _rtx_init(U32 p_func);

extern int k_release_processor(void);
#define release_processor() _release_processor((U32)k_release_processor)
extern int __SVC_0 _release_processor(U32 p_func);

extern void *k_request_memory_block(void);
#define request_memory_block() _request_memory_block((U32)k_request_memory_block)
extern void *_request_memory_block(U32 p_func) __SVC_0;
/* __SVC_0 can also be put at the end of the function declaration */

extern int k_release_memory_block(void *);
#define release_memory_block(p_mem_blk) _release_memory_block((U32)k_release_memory_block, p_mem_blk)
extern int _release_memory_block(U32 p_func, void *p_mem_blk) __SVC_0;

#endif /* !RTX_H_ */
