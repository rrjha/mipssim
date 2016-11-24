#ifndef IFORMAT_H
#define IFORMAT_H

#include "mipssim.h"
#include "datatypes.h"

typedef struct iformat_instr {
        int imm:16;
        unsigned char rt:5;
        unsigned char rs:5;
        unsigned char opcode:6;
}  __attribute__ ((__packed__)) IFormat_Instr;


/* Fetch handle */
void default_handle_itype (const unsigned int instruction);

/* Issue handle */
void itype_issue_two_op(const instr_q_entry *, rs_entry*);

/* Branch - issue */
void branch_issue_one_op(const instr_q_entry *, rs_entry*);
void branch_issue_two_op(const instr_q_entry *, rs_entry*);

/* Load and store issue */
void load_issue(const instr_q_entry *, rs_entry*);
void store_issue(const instr_q_entry *, rs_entry*);

#endif
