#ifndef COMMIT_H
#define COMMIT_H
#include "mipssim.h"


typedef enum RobState {
    EISSUE,
    EEXEC,
    EMEM,
    EWB,
    ECOMMIT
} RobState;

typedef struct reorder_buffer_entry{
	bool busy;
	char instr_string[50];
	INSTRCODE op; //to handle cases for branch, store
	RobState state;
	int value;
	int dest;
	bool br_mispredict;
	unsigned int target_pc;
	unsigned int j_addr;
	unsigned int j_ex_clk;
} rob_entry;

struct rob_q {
    rob_entry rb[MAX_ROB_ENTRIES];
    int front;
    int rear;
    int count;
};

extern struct rob_q rob;

rob_entry* get_top_of_rob();
rob_entry* dequeue_rob();
int create_rob_entry(const char*, INSTRCODE, int);
void commit_instruction();
void reap_instruction();

#endif
