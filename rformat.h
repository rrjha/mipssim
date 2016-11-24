#ifndef RFORMAT_H
#define RFORMAT_H

typedef struct rformat_instr {
        unsigned int funct:6;
        unsigned char sa:5;
        unsigned char rd:5;
        unsigned char rt:5;
        unsigned char rs:5;
        unsigned char opcode:6;
}  __attribute__ ((__packed__)) RFormat_Instr;

/* Fetch handles */
void default_handle_rtype (const unsigned int instruction);

/* Issue handles */
void rtype_issue_zero_op(const instr_q_entry*, rs_entry*);
void rtype_issue_two_op(const instr_q_entry*, rs_entry*);
void rtype_issue_three_op(const instr_q_entry*, rs_entry*);

#endif
