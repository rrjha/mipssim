#ifndef ISSUE_H
#define ISSUE_H

#include "mipssim.h"

/* Reservation station */
typedef struct reservation_station_entry {
	bool busy;
	/* Decoded intruction - Just for printing */
	char instr_string[50];
	INSTRCODE operation;
	int vj;
	int vk;
	int qj;
	int qk;
	int A;
	int value; //holds the result after execution till written in ROB
	int rob_slot;
	int br_pc;
	uint8 cpu_flags;
	bool val_discard; //set when value is not to be committed
}rs_entry;

/*typedef struct register_status_entry {
	bool busy;
	int rob_slot;
}register_status_entry;*/

struct reservation_station {
    rs_entry rs_item[MAX_RS_ENTRIES];
    int front;
    int rear;
    int count;
};

extern struct reservation_station rs;
extern int reg_status_table[GPR_MAX];

void issue_instruction();
rs_entry* get_top_of_rs();
rs_entry* dequeue_rs();


#endif
