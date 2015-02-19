#ifndef K_IPROCESS_H
#define K_IPROCESS_H

#include "rtx.h"

extern int k_delayed_send(int pid, void * message_envelope, int delay);
#define delayed_send(pid, env, delay) _delayed_send((U32)k_delayed_send, process_id, message_envelope, delay)
extern int _delayed_send(U32 p_func, int process_id, void *message_envelope, int delay) __SVC_0;

void init_i_processes(void);
void timeout_i_process(void);
void uart0_i_process(void);

#endif
