#include "mipssim.h"

struct reservation_station rs;
int reg_status_table[GPR_MAX];

int enqueue_rs(rs_entry item)
{
    int retval = -1;
    if(rs.count < MAX_RS_ENTRIES){
        retval = rs.rear;
        rs.rs_item[rs.rear] = item;
        rs.rear = (rs.rear + 1) % MAX_RS_ENTRIES;
        rs.count++;
    }
    return retval;
}

rs_entry* get_top_of_rs()
{
	rs_entry *retval = NULL;
	if(rs.count > 0) {
		retval = &rs.rs_item[rs.front];
		rs.front = (rs.front + 1) % MAX_RS_ENTRIES;
		rs.count--;
	}

	return retval;
}


rs_entry* dequeue_rs()
{
	rs_entry *retval = NULL;
	if(rs.count > 0) {
		retval = &rs.rs_item[rs.front];
		retval->busy = false;
		rs.front = (rs.front + 1) % MAX_RS_ENTRIES;
		rs.count--;
	}

	return retval;
}

int create_rs_entry(const char* instr_str, INSTRCODE op)
{
    int retval = -1;
    rs_entry r;
    if(rs.count < MAX_RS_ENTRIES){
        r.operation = op;
        strcpy(r.instr_string, instr_str);
        r.value = -1; //In exec we must not be looking at this field until ROB is in WB stage
        r.val_discard = false; //once executed the value should be good unless instr has specific req
        r.cpu_flags = 0;
        retval = enqueue_rs(r);
    }
    return retval;
}


void issue_instruction()
{
	int rs_slot=0, rob_slot=-1;
	instr_q_entry *curr_instr = NULL;
	/* Check if reservation Q has space */
	if((rs.count < MAX_RS_ENTRIES) && (rob.count < MAX_ROB_ENTRIES)){
		/* Space in RS & ROB */
		curr_instr = dequeue_iq();
		if (curr_instr) {
            if((curr_instr->internal_code != ENOP) && (curr_instr->internal_code != EBREAK)){
                rs_slot = create_rs_entry(curr_instr->instr_str, curr_instr->internal_code);
                instr_tab[curr_instr->internal_code].issue_handle(curr_instr, &rs.rs_item[rs_slot]);
            }
            else {
                /* NOP and BREAK do not enter RS and go to rob straight */
                rob_slot = create_rob_entry(curr_instr->instr_str, curr_instr->internal_code, -1);
                if (rob_slot >= 0)
                    rob.rb[rob_slot].state = EWB;
            }
        }
	}
}
