#include "mipssim.h"

unsigned int pc = CODE_BASE_ADDR;
struct instr_q gIQ;
struct btb_entry branch_table[BTB_MAX_ENTRY];

void enqueue_iq(instr_q_entry item)
{
	if(gIQ.count < INSTR_Q_SIZE) {
		gIQ.instr[gIQ.rear] = item;
		gIQ.rear = (gIQ.rear + 1) % INSTR_Q_SIZE;
		gIQ.count++;
	}
}

instr_q_entry* dequeue_iq()
{
	instr_q_entry *retval = NULL;
	if(gIQ.count > 0) {
		retval = &gIQ.instr[gIQ.front];
		gIQ.front = (gIQ.front + 1) % INSTR_Q_SIZE;
		gIQ.count--;
	}

	return retval;
}


instr_q_entry* get_top_of_iq()
{
	instr_q_entry *retval = NULL;
	if(gIQ.count > 0)
		retval = &gIQ.instr[gIQ.front];
	return retval;
}


INSTRCODE get_internal_mapping(uint32 instr)
{
	INSTRCODE int_val = opcode_int_map[(instr >> 26) & 0x3F];

	if(int_val == -1) {
		/* Rtype */
		int_val = opcode_rtype_map[instr & 0x3F];
		if((int_val == ESLL) && (((instr >> 6) & 0x1F) == 0 ))
			int_val = ENOP;
	}
	else if ((int_val == EBGEZ) && (((instr >> 16) & 0x1F) == 0))
		int_val = EBLTZ;

	return int_val;
}

void fetch_and_decode()
{
	int i = 0;
	/* Fetch the code at current PC location */
	unsigned int index = (pc-CODE_BASE_ADDR)/sizeof(int);
	unsigned int instr = mips_mem[index];
	if(!gIQ.stop_fetch) {
		/* Now decipher the internal enum for this instruction */
		INSTRCODE internal_code = get_internal_mapping(instr);

		/* Call the specific fetch handler to populate the IQ for this instruction */
		instr_tab[internal_code].if_handle(instr);

		/* Default move PC to next instruction in mem */
		pc += 4;

		if((internal_code >=  EJUMP) && (internal_code <=  EBGTZ)) {
			/* Branch instruction - BTB operations */
			for (i=0; (i < BTB_MAX_ENTRY) && (branch_table[i].valid == true); i++)
				if(branch_table[i].pc_addr == (pc-4)) {
					/* Instruction exists in BTB */
					branch_table[i].last_access = clk_cnt;
					if(branch_table[i].prediction == 1)
						pc = branch_table[i].target_addr;
					break;
				}
		}
		if(internal_code == EBREAK)
			gIQ.stop_fetch = true;
	}

}

