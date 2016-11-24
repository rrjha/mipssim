#include "mipssim.h"

struct rob_q rob;

int enqueue_rob(rob_entry item)
{
    int retval = -1;
    if(rob.count < MAX_ROB_ENTRIES) {
        retval = rob.rear;
        rob.rb[rob.rear] = item;
        rob.rear = (rob.rear + 1) % MAX_ROB_ENTRIES;
        rob.count++;
    }
    return retval;
}

rob_entry* dequeue_rob()
{
	rob_entry *retval = NULL;
	if(rob.count > 0) {
		retval = &rob.rb[rob.front];
		retval->busy = false;
		rob.front = (rob.front + 1) % MAX_ROB_ENTRIES;
		rob.count--;
	}

	return retval;
}


rob_entry* get_top_of_rob()
{
	rob_entry *retval = NULL;
	if(rob.count > 0)
		retval = &rob.rb[rob.front];
	return retval;
}

int create_rob_entry(const char* instr_str, INSTRCODE op, int dest)
{
    int retval = -1;
    rob_entry r;
    if(rob.count < MAX_ROB_ENTRIES){
        r.busy = true;
        strcpy(r.instr_string, instr_str);
        r.dest = dest; //May have -1 for no dest ops
        r.op = op;
        r.state = EISSUE;
        r.value = -1; //invalid till result obtained
        r.br_mispredict = false;
        retval = enqueue_rob(r);
    }
    return retval;
}

void commit_instruction()
{
    int i=0;
    bool j_found = false;
    rob_entry* top = get_top_of_rob();
    if((top != NULL) && (top->state == EWB)){

        top->state = ECOMMIT;

        if(top->op == EJUMP) {
            for (i=0; (i < BTB_MAX_ENTRY) && (branch_table[i].valid == true); i++){
                if(branch_table[i].pc_addr == top->j_addr){
                    j_found = true;
                    break;
                }
            }
            if(j_found == false){
                if(i == BTB_MAX_ENTRY)
                    // BTB full. Find and use LRU index
                    i = find_lru_replacement_index();

                branch_table[i].pc_addr = top->j_addr;
                branch_table[i].last_access = top->j_ex_clk;
                branch_table[i].valid = true;
                branch_table[i].prediction = 1;
                branch_table[i].target_addr = top->target_pc;
                top->br_mispredict = true;
                gIQ.stop_fetch = false;
            }
        }

        if((top->op >= EJUMP) &&
        (top->op <= EBGTZ) &&
        (top->br_mispredict == true)) {
            /* We need to discard everything from ROB and RS */
            /* Mark for flushing the DS at the end of cycle */
            flush_ds = true;
        }

        if(((top->op < EJUMP) ||
        (top->op > EBGTZ)) &&
        (top->op != ESW) &&
        (top->op != ENOP) &&
        (top->op != EBREAK)){
                /* Clear register status table to inform
                 * that value available in reg file now */
                if((top->dest >= 0) &&
                (top->dest < GPR_MAX) &&
                (reg_status_table[top->dest] == rob.front)) {
                    reg_file[top->dest] = top->value;
                    reg_status_table[top->dest] = -1;
                }
        }

        if(top->op == EBREAK)
            end_simulation = true;
    }
}

void reap_instruction()
{
    /* Start from the top and discard instructions that are finished */
    rob_entry* top = get_top_of_rob();

    if((top != NULL) && (top->state == ECOMMIT)) {
        if((top->op != ENOP) && (top->op != EBREAK))
                dequeue_rs();

        dequeue_rob();
    }


    if(flush_ds)
        /* We are done now - reset the flag */
        flush_ds = false;
}
