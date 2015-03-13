#ifndef K_IPROCESS_H
#define K_IPROCESS_H

#include "rtx.h"

#define BUFFER_SIZE 32

void timer_i_process(void);
void uart_i_process(void);

extern int k_delayed_send(int process_id, void *message_envelope, int delay);
#define delayed_send(process_id, message_envelope, delay) _delayed_send((U32)k_delayed_send, process_id, message_envelope, delay)
extern int _delayed_send(U32 p_func, int process_id, void *message_envelope, int delay) __SVC_0;

void removeMessagesFromTimerProcess(int pid);
int get_system_time(void);

#endif
