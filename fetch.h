#ifndef FETCH_H
#define FETCH_H

#include "mipssim.h"
#include "datatypes.h"

typedef struct instr_q_entry {
	union {
		JFormat_Instr j_instr;
		IFormat_Instr i_instr;
		RFormat_Instr r_instr;
	};
	int br_pc; // PC for branch instruction so that we can update BTB later
	char instr_str[30];
	INSTRCODE internal_code;
}instr_q_entry;

struct instr_q {
	instr_q_entry instr[INSTR_Q_SIZE];
	bool stop_fetch;
	unsigned int front;
	unsigned int rear;
	unsigned int count;
};

/* For Branch instructions issue depends on BTB */
struct btb_entry {
	unsigned int pc_addr;
	unsigned int target_addr;
	unsigned int last_access;
	int prediction;
	bool valid;
};

void fetch_and_decode();
INSTRCODE get_internal_mapping(uint32);
void enqueue_iq(instr_q_entry);
instr_q_entry* get_top_of_iq();
instr_q_entry* dequeue_iq();

extern unsigned int pc;
extern struct btb_entry branch_table[BTB_MAX_ENTRY];
extern struct instr_q gIQ;

#endif
