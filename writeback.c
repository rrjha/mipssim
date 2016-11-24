#include "mipssim.h"

struct cdb_pipe cdb;

void add_to_cdb_pipe(unsigned int slot, int value)
{
    cdb.cdb_item[cdb.num_entries].rob_slot = slot;
    cdb.cdb_item[cdb.num_entries].value = value;
    // Point to next item, if any
    cdb.num_entries++;
}

void cdb_broadcast()
{
    int i, j;
    for(i=0; i < cdb.num_entries; i++) {
        for(j=0; j < rs.count; j++){
            if(rs.rs_item[(rs.front+j)%MAX_RS_ENTRIES].qj == cdb.cdb_item[i].rob_slot) {
                rs.rs_item[(rs.front+j)%MAX_RS_ENTRIES].vj = cdb.cdb_item[i].value;
                rs.rs_item[(rs.front+j)%MAX_RS_ENTRIES].qj = -1;
            }
            if(rs.rs_item[(rs.front+j)%MAX_RS_ENTRIES].qk == cdb.cdb_item[i].rob_slot) {
                rs.rs_item[(rs.front+j)%MAX_RS_ENTRIES].vk = cdb.cdb_item[i].value;
                rs.rs_item[(rs.front+j)%MAX_RS_ENTRIES].qk = -1;
            }
        }
    }
    /* Reset the CDB */
    cdb.num_entries = 0;
}

void writeback_instruction()
{
    int i;
    // Flush all cdb items pending in cdb pipe for waiting instructions
    // Note we are publishing cdb items of previous cycle to simulate
    // cycle delay in writeback and execute of dependent instruction
    cdb_broadcast();

    for(i=0; i < rs.count; i++){
        /* Store */
        if((rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state == EEXEC) &&
            (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation == ESW) &&
            (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].qk == -1)) {
            rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].value = rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].vk;
            rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state = EWB;
        }
        /* Load */
        else if((rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state == EMEM) &&
            (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation == ELW)) {
            rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].value = rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].value;
            rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state = EWB;
            // Now add items writing back in this cycle - to be flushed next cycle
            add_to_cdb_pipe(rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot, rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].value);
        }
        /* Rest */
        else if((rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state == EEXEC) &&
            (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation != ESW) &&
            (rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].operation != ELW)) {
            if(rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].val_discard == false) //No CPU flag
                rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].value = rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].value;
            rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state = EWB;
            add_to_cdb_pipe(rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot, rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].value);
        }
    }
}
