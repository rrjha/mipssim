#ifndef MIPSSIM_H
#define MIPSSIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "datatypes.h"
#include "mipsloader.h"
#include "iformat.h"
#include "rformat.h"
#include "jformat.h"
#include "fetch.h"
#include "issue.h"
#include "execute.h"
#include "writeback.h"
#include "commit.h"

// Handlers for various stages
//typedef int (*writeback_handle)(instr_q_entry*);
//typedef int (*commit_handle)(reorder_buffer_entry*);

typedef struct Instruction_Table_Entry {
	char instr_mnemonic[10];
	fetch_decode_handle if_handle;
	rs_issue_handle issue_handle;
	execute_handle exec_handle;
//	writeback_handle wb_handle;
//	commit_handle cmt_handle;
}Instruction_Table_Entry;

extern Instruction_Table_Entry instr_tab[EINSTRUCTION_MAX];
extern int opcode_int_map[64];
extern int opcode_rtype_map[64];
extern unsigned int clk_cnt;
extern bool flush_ds;
extern bool end_simulation;
#endif
