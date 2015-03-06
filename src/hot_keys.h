#ifndef HOT_KEYS_H
#define HOT_KEYS_H

#define DEBUG_HOTKEY_1 '!'
#define DEBUG_HOTKEY_2 '@'
#define DEBUG_HOTKEY_3 '#'

#include "k_rtx.h"

void process_hot_key(char c);

void print_queue(void);			//depending on queue implementation, should take param
void print_process(pcb *proc);


#endif
