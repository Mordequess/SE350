#ifndef K_SYSTEM_PROC_H
#define K_SYSTEM_PROC_H

//Both of these constants are flexible but they must be consts
#define MAX_COMMAND_LENGTH 15
#define MAX_COMMANDS 10

typedef struct registered_command {
	int  process_id;
	char command[MAX_COMMAND_LENGTH];
} registered_command;

void kcd_proc(void);
void crt_proc(void);
void wall_clock_proc(void);

//helper functions

//outputs converted value to char* parameter
void time_to_hms(int sss, char* target);

//parameter is a time in the form (hh:mm:ss)
int time_to_sss(char* hms);

#endif
