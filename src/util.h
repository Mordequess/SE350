#include "k_rtx.h"
// for access to msgbuf struct

#ifndef UTIL_H
#define UTIL_H

/** struct for local message queue for stress test C **/
typedef struct l_msg_t {
	struct l_msg_t* next;
	msgbuf* content;
} lmsg;

void assert(int, unsigned char *);
void copy_string(char[], char[]);
int strings_are_equal(char[], char[]);
int str_len(char[]);
int get_int_from_string(char *);

#endif
