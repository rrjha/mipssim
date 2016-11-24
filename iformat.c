#include <string.h>
#include "mipssim.h"

void default_handle_itype (const unsigned int instruction) {
	instr_q_entry item;
	char *sPtr = &(item.instr_str[0]);
	item.internal_code = get_internal_mapping(instruction);

	item.i_instr.imm = (instruction & 0x0000FFFF);
	item.i_instr.rt = ((instruction >> 16) & 0x0000001F);
	item.i_instr.rs = ((instruction >> 21) & 0x0000001F);
	item.i_instr.opcode = ((instruction >> 26) & 0x0000003F);

    strcpy(item.instr_str, instr_tab[item.internal_code].instr_mnemonic);
	sPtr += strlen(instr_tab[item.internal_code].instr_mnemonic);

        switch(item.internal_code) {
                case EBLTZ:
                        /* BLTZ */
                case EBGEZ:
                        /* BGEZ */
               	case EBLEZ:
                        /* BLEZ */
                case EBGTZ:
                        /* BGTZ */
                        sprintf(sPtr, " R%u, #%d", item.i_instr.rs, (int)((int)(item.i_instr.imm) << 2));
                        /* record PC for updating BTB later */
                        item.br_pc = pc;
                        break;
                case EBEQ:
                        /* BEQ */
                case EBNE:
                        /* BNE */
                        sprintf(sPtr, " R%u, R%u, #%d", item.i_instr.rs, item.i_instr.rt, (int)((int)(item.i_instr.imm) << 2));
                        /* record PC for updating BTB later */
                        item.br_pc = pc;
                        break;
                case EADDI:
                        /* ADDI */
                case EADDIU:
                        /* ADDIU */
                case ESLTI:
                        /* SLTI */
                case ESLTIU:
                        /* SLTIU */
                        sprintf(sPtr, " R%u, R%u, #%d", item.i_instr.rt, item.i_instr.rs, (int)(item.i_instr.imm));
                        break;
                case ELW:
                        /* LW */
                case ESW:
                        /* SW */
                        sprintf(sPtr, " R%u, %d(R%u)", item.i_instr.rt, (int)(item.i_instr.imm), item.i_instr.rs);
                        break;
                default:
                        /* We should not be here - log it as error */
                        printf("Unhandled I-Format Opcode - %s - %u\n", instr_tab[item.internal_code].instr_mnemonic, item.i_instr.opcode);
        }

	enqueue_iq(item);
}

void itype_issue_two_op(const instr_q_entry *instr, rs_entry* rs_item)
{
    rs_item->rob_slot = create_rob_entry(instr->instr_str, instr->internal_code, instr->i_instr.rt);
    if(rs_item->rob_slot != -1) {
        rs_item->busy = true;
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
        rs_item->A = instr->i_instr.imm;

        /* Set reg status table for destination as busy */
        reg_status_table[instr->i_instr.rt] = rs_item->rob_slot;
    }
}

