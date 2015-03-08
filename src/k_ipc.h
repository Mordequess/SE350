#ifndef K_IPC_H
#define K_IPC_H

#include "rtx.h"
#include "k_rtx.h"
#include "k_process.h"

/* -------- QUEUES FOR MESSAGES -------- */

void m_enqueue(int sender_id, message* element);
message* m_dequeue(int destination_id);
void m_remove_queue_node(int sender_id, message* element);
U32 m_is_empty(int sender_id);
U32 m_any_messages_from_sender(int destination_id, int sender_id);

/* ----- Functions ----- */

extern int k_send_message(int process_id, void* message_envelope);
#define send_message(pid, env) _send_message((U32)k_send_message, pid, env)
extern int _send_message(U32 p_func, int pid, void* env) __SVC_0;
																												
extern void* k_receive_message(int* sender_id);
#define receive_message(sender) _receive_message((U32)k_receive_message, sender)
extern void* _receive_message(U32 p_func, int* sender) __SVC_0;

extern int k_delayed_send(int process_id, void *message_envelope, int delay);
#define delayed_send(process_id, message_envelope, delay) _delayed_send((U32)k_delayed_send, process_id, message_envelope, delay)
extern int _delayed_send(U32 p_func, int process_id, void *message_envelope, int delay) __SVC_0;

#endif
