#include "mipssim.h"

/* EX declarations */

void execute_instruction();
void handle_nop(rs_entry *);
void handle_sll(rs_entry *);
void handle_srl(rs_entry *);
void handle_sra(rs_entry *);
void handle_break(rs_entry *);
void handle_add(rs_entry *);
void handle_addu(rs_entry *);
void handle_sub(rs_entry *);
void handle_subu(rs_entry *);
void handle_and(rs_entry *);
void handle_or(rs_entry *);
void handle_xor(rs_entry *);
void handle_nor(rs_entry *);
void handle_slt(rs_entry *);
void handle_sltu(rs_entry *);
void handle_jump(rs_entry *);
void handle_bltz(rs_entry *);
void handle_bgez(rs_entry *);
void handle_beq(rs_entry *);
void handle_bne(rs_entry *);
void handle_blez(rs_entry *);
void handle_bgtz(rs_entry *);
void handle_addi(rs_entry *);
void handle_addiu(rs_entry *);
void handle_slti(rs_entry *);
void handle_sltiu(rs_entry *);
void handle_load(rs_entry *);
void handle_store(rs_entry *);

/* Mem stage */
void mem_access();

/* WB stage */
void writeback_instruction();

int find_lru_replacement_index();

extern int bit_mask_arr[32];
