#ifndef HOT_KEYS_H
#define HOT_KEYS_H

#define DEBUG_HOTKEY_1 '!'
#define DEBUG_HOTKEY_2 '@'
#define DEBUG_HOTKEY_3 '#'
#define DEBUG_HOTKEY_4 '$'

#include "k_rtx.h"

void process_hot_key(char c);

void print_queue(pcb*);
void print_process(pcb*);


#endif
