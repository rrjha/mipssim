#include "mipssim.h"

void branch_issue_no_op(const instr_q_entry *instr, rs_entry* rs_item)
{
    /* Jump */
    rs_item->rob_slot = create_rob_entry(instr->instr_str, instr->internal_code, -1); //no dest
    if(rs_item->rob_slot != -1) {
        rs_item->busy = true;
        rs_item->qj = -1; //Invalid - Not waiting for any rob entry
        rs_item->qk = -1;
        rs_item->vj = 0;
        rs_item->vk = 0;
        rs_item->A = (((int)(instr->j_instr.jump_addr)) << 2);
        rs_item->br_pc = instr->br_pc;
    }
}

void branch_issue_one_op(const instr_q_entry *instr, rs_entry* rs_item)
{
    rs_item->rob_slot = create_rob_entry(instr->instr_str, instr->internal_code, -1);
    if(rs_item->rob_slot != -1) {
        rs_item->busy = true;
        rs_item->A = (int)((int)(instr->i_instr.imm) << 2);
        rs_item->br_pc = instr->br_pc;
        rs_item->qk = -1;
        rs_item->vk = 0;

        rs_item->qj = reg_status_table[instr->i_instr.rs]; //-1 if not busy else value of rob
        if(rs_item->qj == -1)
            /* Value in register file */
            rs_item->vj = reg_file[instr->i_instr.rs];
        else if((rs_item->qj != -1) && (rob.rb[rs_item->qj].state > EMEM)) {
            /* Instr ready in ROB- Read from ROB */
            rs_item->vj = rob.rb[rs_item->qj].value;
            rs_item->qj = -1;
        }
    }
}

void branch_issue_two_op(const instr_q_entry *instr, rs_entry* rs_item)
{
    rs_item->rob_slot = create_rob_entry(instr->instr_str, instr->internal_code, -1);
    if(rs_item->rob_slot != -1) {
        rs_item->busy = true;
        rs_item->A = (int)((int)(instr->i_instr.imm) << 2);
        rs_item->br_pc = instr->br_pc;

        rs_item->qj = reg_status_table[instr->i_instr.rs]; //-1 if not busy else value of rob
        if(rs_item->qj == -1)
            /* Value in register file */
            rs_item->vj = reg_file[instr->i_instr.rs];
        else if((rs_item->qj != -1) && (rob.rb[rs_item->qj].state > EMEM)) {
            /* Instr ready in ROB- Read from ROB */
            rs_item->vj = rob.rb[rs_item->qj].value;
            rs_item->qj = -1;
        }

        rs_item->qk = reg_status_table[instr->i_instr.rt]; //-1 if not busy else value of rob
        if(rs_item->qk == -1)
            /* Value in register file */
            rs_item->vk = reg_file[instr->i_instr.rt];
        else if((rs_item->qk != -1) && (rob.rb[rs_item->qk].state > EMEM)) {
            /* Instr ready in ROB- Read from ROB */
            rs_item->vk = rob.rb[rs_item->qk].value;
            rs_item->qk = -1;
        }
    }
}
