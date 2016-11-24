#include <stdio.h>
#include <string.h>
#include "mipssim.h"

void default_handle_jtype (const unsigned int instruction) {
	instr_q_entry item;
	char *bcd_addr = &(item.instr_str[0]);
	item.internal_code = get_internal_mapping(instruction);
	item.br_pc = pc;

	item.j_instr.jump_addr = (instruction & 0x03FFFFFF);
	item.j_instr.opcode = ((instruction >> 26) & 0x0000003F);

    strcpy(item.instr_str, instr_tab[item.internal_code].instr_mnemonic);
	bcd_addr += strlen(instr_tab[item.internal_code].instr_mnemonic);

    if(item.j_instr.opcode == 2)
            sprintf(bcd_addr, " #%u", ((pc & 0xF0000000) | (((int)(item.j_instr.jump_addr)) << 2)) & 0x0FFFFFFF);
    else
            printf("Unhandled J-Format Opcode - %s - %u\n",  instr_tab[item.internal_code].instr_mnemonic, item.j_instr.opcode);

	enqueue_iq(item);
}
