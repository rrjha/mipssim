#include "mipssim.h"

#define MSB 0x10000000
#define OVERFLOW 0x01

int bit_mask_arr[32];

int find_lru_replacement_index()
{
    int i=0, min = branch_table[0].last_access, ret = 0;
    for(i = 0; i < BTB_MAX_ENTRY; i++){
        if(branch_table[i].last_access < min){
            /* Older than our last oldest recorded so update oldest */
            min = branch_table[i].last_access;
            ret = i;
        }
    }

    return ret;
}

void btb_update(rs_entry *rs_item, int taken_pc)
{
    int i=0;
    bool br_found = false;
    for (i=0; (i < BTB_MAX_ENTRY) && (branch_table[i].valid == true); i++) { //fully associative cache so can't have holes
        if(branch_table[i].pc_addr == rs_item->br_pc) {
            /* Instruction exists in BTB */
            if((rs_item->value == 1) && (branch_table[i].prediction < 1)) {
                /* Misprediction */
                /* Update PC */
                //pc = taken_pc;
                /* Set misprediction for commit */
                rob.rb[rs_item->rob_slot].br_mispredict = true;
                rob.rb[rs_item->rob_slot].target_pc = taken_pc;
                /* Now update the BTB */
                //branch_table[i].target_addr = taken_pc;
                branch_table[i].prediction = 1;
                // if we stopped fetching start again
                gIQ.stop_fetch = false;
            }
            else if((rs_item->value == 0) && (branch_table[i].prediction == 1)) {
                /* Misprediction */
                /* Update PC */
                //pc = rs_item->br_pc + 4;
                /* Set misprediction for commit */
                rob.rb[rs_item->rob_slot].br_mispredict = true;
                rob.rb[rs_item->rob_slot].target_pc = rs_item->br_pc + 4;
                /* Now update the BTB */
                //branch_table[i].target_addr = rs_item->br_pc + 4;
                branch_table[i].prediction = 0;
                // if we stopped fetching start again
                gIQ.stop_fetch = false;
            }
            br_found = true;
            break;
        }
    }//end for

    if(br_found == false){
        if(i == BTB_MAX_ENTRY)
            // BTB full. Find and use LRU index
            i = find_lru_replacement_index();

        branch_table[i].pc_addr = rs_item->br_pc;
        branch_table[i].last_access = clk_cnt;
        branch_table[i].valid = true;
        branch_table[i].prediction = rs_item->value;
        branch_table[i].target_addr = taken_pc;
        if(rs_item->value == 1) {
            //pc = taken_pc;
            rob.rb[rs_item->rob_slot].br_mispredict = true;
            rob.rb[rs_item->rob_slot].target_pc = taken_pc;
            gIQ.stop_fetch = false;
        }
    }
}

void execute_instruction()
{
    /* Iterate through the RS entries to find instructions to execute */
    int i=0;

    for(i=0; i < rs.count; i++){
        if((rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state == EISSUE) &&
        (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].qj == -1)) {
            if ((rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation == ESW) ||
            (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].qk == -1)) {
                rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state = EEXEC;
                if((rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation >= EJUMP) &&
                (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation <= EBGTZ))
                    /* Branch skips mem and writeback */
                    rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state = EWB;
                /* Found an instruction to execute */
                instr_tab[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation].exec_handle(&rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES]);
            }
        }
    }
}

void handle_nop(rs_entry *rs_item)
{
    /* Nothing to execute */
}

void handle_sll(rs_entry *rs_item)
{
    /* shift left */
    rs_item->value = (rs_item->vk << rs_item->A);
}

void handle_srl(rs_entry *rs_item)
{
    /* Logical right shift */
    rs_item->value = (rs_item->vk >> rs_item->A) & bit_mask_arr[rs_item->A];
}

void handle_sra(rs_entry *rs_item)
{
    /* Arithmetic right shift */
    rs_item->value = (rs_item->vk >> rs_item->A);
}

void handle_break(rs_entry *rs_item)
{
    /* Nothing to execute - Just mark it complete */
}

void handle_add(rs_entry *rs_item)
{
    /* Add - with overflow trap */
    rs_item->value = rs_item->vj + rs_item->vk;

    /* Check if overflow generated */
    if(((rs_item->vj & MSB) == (rs_item->vk & MSB)) &&
       ((rs_item->vj & MSB) != (rs_item->value & MSB))) {
        rs_item->val_discard = true;
        rs_item->cpu_flags |= OVERFLOW;
    }
}

void handle_addu(rs_entry *rs_item)
{
    /* Add - ignore overflow */
    rs_item->value = rs_item->vj + rs_item->vk;
}

void handle_sub(rs_entry *rs_item)
{
    int temp = 0 - rs_item->vk;
    /* Sub - with overflow trap */
    rs_item->value = rs_item->vj + temp;

    /* Check if overflow generated */
    if(((rs_item->vj & MSB) == (temp & MSB)) &&
       ((rs_item->vj & MSB) != (rs_item->value & MSB))) {
        rs_item->val_discard = true;
        rs_item->cpu_flags |= OVERFLOW;
    }
}

void handle_subu(rs_entry *rs_item)
{
    /* Sub - ignore overflow */
    rs_item->value = rs_item->vj - rs_item->vk;
}

void handle_and(rs_entry *rs_item)
{
    rs_item->value = rs_item->vj & rs_item->vk;
}

void handle_or(rs_entry *rs_item)
{
    rs_item->value = rs_item->vj | rs_item->vk;
}

void handle_xor(rs_entry *rs_item)
{
    rs_item->value = rs_item->vj ^ rs_item->vk;
}

void handle_nor(rs_entry *rs_item)
{
    rs_item->value = ~(rs_item->vj | rs_item->vk);
}

void handle_slt(rs_entry *rs_item)
{
    rs_item->value = (rs_item->vj < rs_item->vk);
}

void handle_sltu(rs_entry *rs_item)
{
    rs_item->value = ((unsigned int)rs_item->vj < (unsigned int)rs_item->vk);
}

void handle_jump(rs_entry *rs_item)
{
    int j_pc = ((rs_item->br_pc & 0xF0000000) | rs_item->A) & 0x0FFFFFFF;

    /* Set PC to new jump address */
    rob.rb[rs_item->rob_slot].target_pc = j_pc;
    rob.rb[rs_item->rob_slot].j_addr = rs_item->br_pc;
    rob.rb[rs_item->rob_slot].j_ex_clk = clk_cnt;
}

void handle_bltz(rs_entry *rs_item)
{
    int ta;
    rs_item->value = (rs_item->vj < 0);
    ta = (rs_item->br_pc + 4 + rs_item->A);
    btb_update(rs_item, ta);
}

void handle_bgez(rs_entry *rs_item)
{
    int ta;
    rs_item->value = (rs_item->vj >= 0);
    ta = (rs_item->br_pc + 4 + rs_item->A);
    btb_update(rs_item, ta);
}

void handle_beq(rs_entry *rs_item)
{
    int ta;
    rs_item->value = (rs_item->vj == rs_item->vk);
    ta = (rs_item->br_pc + 4 + rs_item->A);
    btb_update(rs_item, ta);
}

void handle_bne(rs_entry *rs_item)
{
    int ta;
    rs_item->value = (rs_item->vj != rs_item->vk);
    ta = (rs_item->br_pc + 4 + rs_item->A);
    btb_update(rs_item, ta);
}

void handle_blez(rs_entry *rs_item)
{
    int ta;
    rs_item->value = (rs_item->vj <= 0);
    ta = (rs_item->br_pc + 4 + rs_item->A);
    btb_update(rs_item, ta);
}

void handle_bgtz(rs_entry *rs_item)
{
    int ta;
    rs_item->value = (rs_item->vj > 0);
    ta = (rs_item->br_pc + 4 + rs_item->A);
    btb_update(rs_item, ta);
}

void handle_addi(rs_entry *rs_item)
{
    /* Add - with overflow trap */
    rs_item->value = rs_item->vj + rs_item->A;

    /* Check if overflow generated */
    if(((rs_item->vj & MSB) == (rs_item->A & MSB)) &&
       ((rs_item->vj & MSB) != (rs_item->value & MSB))) {
        rs_item->val_discard = true;
        rs_item->cpu_flags |= OVERFLOW;
    }
}

void handle_addiu(rs_entry *rs_item)
{
    rs_item->value = rs_item->vj + rs_item->A;
}

void handle_slti(rs_entry *rs_item)
{
    rs_item->value = (rs_item->vj < rs_item->A);
}

void handle_sltiu(rs_entry *rs_item)
{
    rs_item->value = ((unsigned int)rs_item->vj < (unsigned int)rs_item->A);
}

void handle_load(rs_entry *rs_item)
{
    /* Go ahead iff no store ahead with address calc not done */
    int i=rs_item->rob_slot;
    for (; i != ((rob.front + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES); i = (i + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES)
        if((rob.rb[i].op == ESW) && (rob.rb[i].state == EISSUE)) {
            /* We need to wait for this store to generate an address */
            rob.rb[rs_item->rob_slot].state = EISSUE; //go back and wait
            break;
        }
    if (i == ((rob.front + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES))
        /* Good to go - calc the address and put in line for mem */
        /* We will check for mem conflict in mem stage */
        rs_item->A = rs_item->vj + rs_item->A;
}

void handle_store(rs_entry *rs_item)
{
    /* Check all load/store ahead have address ready */
    int i=rs_item->rob_slot;
    for (; i != ((rob.front + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES); i = (i + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES)
        if(((rob.rb[i].op == ELW) || (rob.rb[i].op == ESW)) && (rob.rb[i].state == EISSUE)) {
            /* We need to wait for address generation of instr ahead */
            rob.rb[rs_item->rob_slot].state = EISSUE; //go back and wait
            break;
        }
    if (i == ((rob.front + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES)){
        /* Good to go - calc the address and put in line for mem */
        /* We will check for mem conflict in mem stage */
        rs_item->A = rs_item->vj + rs_item->A;
        rob.rb[rs_item->rob_slot].dest = rs_item->A;
        /* Check if 2nd operand also read, if so fast forward to commit */
        if(rs_item->qk == -1){
            rob.rb[rs_item->rob_slot].value = rs_item->vk;
            rob.rb[rs_item->rob_slot].state = EWB;
        }
    }
}

void mem_access()
{
    /* Memory access stage for load and store */
    int i=0, j, mem_idx;

    /* Iterate through the RS and find load instructions *
     * that are in EX and waiting for Mem. Or Stores that *
     * are in commit and waiting for Mem */
    for(i=0; i < rs.count; i++){
        if((rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state == EEXEC) &&
        (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation != ESW)) {
            if (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation == ELW) {
                /* Iterate through the rob and confirm no early store with same address */
                /* TODO - Change this to wait for only latest store */
                for (j=rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot;
                j != ((rob.front + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES); j = (j + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES)
                    if((rob.rb[j].op == ESW) &&
                    (rob.rb[j].dest == rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].A))
                        break;


                if(j == ((rob.front + MAX_ROB_ENTRIES - 1)%MAX_ROB_ENTRIES)){
                        /* Good to go - no matching store - Fetch mem */
                        mem_idx = (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].A - CODE_BASE_ADDR)/sizeof(int);
                        rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].value = mips_mem[mem_idx];
                        rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state = EMEM;
                }
            }
            //else
                /* Other instruction just fall through */
                //rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state = EMEM;
        }
        else if((rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].busy == true) &&
            (rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state == ECOMMIT) &&
            (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation == ESW)) {
            /* Ready and top of the rob Q */
            /* Store to mem location */
            mem_idx = (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].A - CODE_BASE_ADDR)/sizeof(int);
            mips_mem[mem_idx] = rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].value;
            /* Dq from rob and remove from RS */
            dequeue_rob();
            dequeue_rs();
        }
    }
}
