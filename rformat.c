#include <stdio.h>
#include <string.h>
#include "mipssim.h"


void default_handle_rtype (const unsigned int instruction) {
	instr_q_entry item;
	char *instr_str = &(item.instr_str[0]);
	item.internal_code = get_internal_mapping(instruction);

        item.r_instr.funct = (instruction & 0x0000003F);
        item.r_instr.sa = ((instruction >> 6) & 0x0000001F);
        item.r_instr.rd = ((instruction >> 11) & 0x0000001F);
        item.r_instr.rt = ((instruction >> 16) & 0x0000001F);
        item.r_instr.rs = ((instruction >> 21) & 0x0000001F);
        item.r_instr.opcode = ((instruction >> 26) & 0x0000003F);

        strcpy(item.instr_str, instr_tab[item.internal_code].instr_mnemonic);
        instr_str += strlen(instr_tab[item.internal_code].instr_mnemonic);

        switch(item.internal_code) {
                case ENOP:
                     /* NOP */
                case EBREAK:
                        /* BREAK */
			break;
		case ESLL:
			/* SLL */
                case ESRL:
                        /* SRL */
                case ESRA:
                        /* SRA */
                        sprintf(instr_str, " R%u, R%u, #%u", item.r_instr.rd, item.r_instr.rt, item.r_instr.sa);
                        break;
                case EADD:
                        /* ADD */
                case EADDU:
                        /* ADDU */
                case ESUB:
                        /* SUB */
                case ESUBU:
                        /* SUBU */
                case EAND:
                        /* AND */
                case EOR:
                        /* OR */
                case EXOR:
                        /* XOR */
                case ENOR:
                        /* NOR */
                case ESLT:
                        /* SLT */
                case ESLTU:
                        /* SLTU */
                        sprintf(instr_str, " R%u, R%u, R%u", item.r_instr.rd, item.r_instr.rs, item.r_instr.rt);
                        break;
                default:
                        /* We should not be here - log it as error */
                        printf("Unhandled R-Format Opcode - %u - %u\n", item.r_instr.opcode, item.r_instr.funct);
        }

	enqueue_iq(item);
}

void rtype_issue_zero_op(const instr_q_entry *instr, rs_entry* rs_item)
{
    rs_item->rob_slot = create_rob_entry(instr->instr_str, instr->internal_code, -1); //no dest
    if(rs_item->rob_slot != -1) {
        rs_item->busy = true;
        rs_item->qj = -1; //Invalid - Not waiting for any rob entry
        rs_item->qk = -1;
        rs_item->vj = 0;
        rs_item->vk = 0;
        rs_item->A = 0; //Ignore
    }
}

void rtype_issue_two_op(const instr_q_entry *instr, rs_entry* rs_item)
{
    rs_item->rob_slot = create_rob_entry(instr->instr_str, instr->internal_code, instr->r_instr.rd);
    if(rs_item->rob_slot != -1) {
        rs_item->busy = true;

        rs_item->qj = -1;
        rs_item->vj = 0;

        rs_item->qk = reg_status_table[instr->r_instr.rt]; //-1 if not busy else value of rob
        if(rs_item->qk == -1)
            /* Value in register file */
            rs_item->vk = reg_file[instr->r_instr.rt];
        else if((rs_item->qk != -1) && (rob.rb[rs_item->qk].state > EMEM)) {
            /* Instr ready in ROB- Read from ROB */
            rs_item->vk = rob.rb[rs_item->qk].value;
            rs_item->qk = -1;
        }
        rs_item->A = instr->r_instr.sa;
        /* Set reg status table for destination as busy */
        reg_status_table[instr->r_instr.rd] = rs_item->rob_slot;
    }
}

void rtype_issue_three_op(const instr_q_entry *instr, rs_entry* rs_item)
{
    rs_item->rob_slot = create_rob_entry(instr->instr_str, instr->internal_code, instr->r_instr.rd);
    if(rs_item->rob_slot != -1) {
        rs_item->busy = true;

        rs_item->qj = reg_status_table[instr->r_instr.rs]; //-1 if not busy else value of rob
        if(rs_item->qj == -1)
            /* Value in register file */
            rs_item->vj = reg_file[instr->r_instr.rs];
        else if((rs_item->qj != -1) && (rob.rb[rs_item->qj].state > EMEM)) {
            /* Instr ready in ROB- Read from ROB */
            rs_item->vj = rob.rb[rs_item->qj].value;
            rs_item->qj = -1;
        }

        rs_item->qk = reg_status_table[instr->r_instr.rt]; //-1 if not busy else value of rob
        if(rs_item->qk == -1)
            /* Value in register file */
            rs_item->vk = reg_file[instr->r_instr.rt];
        else if((rs_item->qk != -1) && (rob.rb[rs_item->qk].state > EMEM)) {
            /* Instr ready in ROB- Read from ROB */
            rs_item->vk = rob.rb[rs_item->qk].value;
            rs_item->qk = -1;
        }
        reg_status_table[instr->r_instr.rd] = rs_item->rob_slot;
        rs_item->A = 0; //Ignore
    }
}
