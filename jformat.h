#ifndef JFORMAT_H
#define JFORMAT_H

#include "mipssim.h"
#include "datatypes.h"

typedef struct jformat_instr {
        int jump_addr:26;
        unsigned char opcode:6;
}  __attribute__ ((__packed__)) JFormat_Instr;

/* Fetch handle */
void default_handle_jtype (const unsigned int instruction);

/* Issue handle */
void branch_issue_no_op(const instr_q_entry *, rs_entry *);

#endif
