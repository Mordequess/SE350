#ifndef K_IPC_H
#define K_IPC_H

#include "rtx.h"
#include "k_rtx.h"
#include "k_process.h"

#define DEFAULT 0 /* A general purpose message.*/
#define KCD_REG 1 /* A message to register a command with the Keyboard Command Decoder Process */

//their required message format
typedef struct msgbuf {
	int mtype; /* user defined message type. One of the two constants above. */
	char mtext[1]; /* body of the message */
} msgbuf;

//msgbuf wrapper containing the rest of the information we need for message passing
typedef struct message {
	msgbuf* message_envelope;
	int sender_id;
	int destination_id;
	struct message* mp_next;
} message;

message* message_new(int sender, int destination, msgbuf* envelope) {
	message* m;
	m->sender_id = sender;
	m->destination_id = destination;
	m->message_envelope = envelope;
	m->mp_next = NULL;
	return m;
}

/* -------- QUEUES FOR MESSAGES -------- */

void m_enqueue(message** targetQueue, message* element);
message* m_dequeue(message** targetQueue);
void m_remove_queue_node(message** targetQueue, message* element);
U32 m_is_empty(message* targetQueue);
U32 m_any_messages(message* targetQueue, int destination_id);

/* ----- Functions ----- */

extern int k_send_message(int destination_id, void* message_envelope);
#define send_message(pid, env) _send_message((U32)k_send_message, pid, env)
extern int _send_message(U32 p_func, int pid, void* env) __SVC_0;
																												
extern void* k_receive_message(int* sender_id);
#define receive_message(sender) _receive_message((U32)k_receive_message, sender)
extern void* _receive_message(U32 p_func, int* sender) __SVC_0;

extern int k_delayed_send(int pid, void * message_envelope, int delay);
#define delayed_send(pid, env, delay) _delayed_send((U32)k_delayed_send, process_id, message_envelope, delay)
extern int _delayed_send(U32 p_func, int process_id, void *message_envelope, int delay) __SVC_0;

#endif
