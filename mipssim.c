#include <stdio.h>
#include <unistd.h>
#include "mipssim.h"

/* Internal mappings */
int opcode_int_map[64];
int opcode_rtype_map[64];
unsigned int clk_cnt=1;
bool flush_ds = false;
bool end_simulation = false;

Instruction_Table_Entry instr_tab[EINSTRUCTION_MAX] = {
	/* Internal Mapping */        /* Mnemonic */          /* IF handle */   /* Issue handle */      /* Execute handle */
	{/* ENOP */			            "NOP",			default_handle_rtype,   rtype_issue_zero_op,    handle_nop  },
	{/* ESLL */			            "SLL",			default_handle_rtype,   rtype_issue_two_op,     handle_sll  },
	{/* ESRL */			            "SRL",			default_handle_rtype,   rtype_issue_two_op,     handle_srl  },
	{/* ESRA */			            "SRA",			default_handle_rtype,   rtype_issue_two_op,     handle_sra  },
	{/* EBREAK */		            "BREAK",		default_handle_rtype,   rtype_issue_zero_op,    handle_break},
	{/* EADD */			            "ADD",			default_handle_rtype,   rtype_issue_three_op,   handle_add  },
	{/* EADDU */		            "ADDU",			default_handle_rtype,   rtype_issue_three_op,   handle_addu },
	{/* ESUB */			            "SUB",			default_handle_rtype,   rtype_issue_three_op,   handle_sub  },
	{/* ESUBU */		            "SUBU",			default_handle_rtype,   rtype_issue_three_op,   handle_subu },
	{/* EAND */			            "AND",			default_handle_rtype,   rtype_issue_three_op,   handle_and  },
	{/* EOR */			            "OR",			default_handle_rtype,   rtype_issue_three_op,   handle_or   },
	{/* EXOR */			            "XOR",			default_handle_rtype,   rtype_issue_three_op,   handle_xor  },
	{/* ENOR */			            "NOR",			default_handle_rtype,   rtype_issue_three_op,   handle_nor  },
	{/* ESLT */			            "SLT",			default_handle_rtype,   rtype_issue_three_op,   handle_slt  },
	{/* ESLTU */		            "SLTU",			default_handle_rtype,   rtype_issue_three_op,   handle_sltu },
	{/* EJUMP */			        "J",			default_handle_jtype,   branch_issue_no_op,     handle_jump },
	{/* EBLTZ */			        "BLTZ",			default_handle_itype,   branch_issue_one_op,    handle_bltz },
	{/* EBGEZ */			        "BGEZ",			default_handle_itype,   branch_issue_one_op,    handle_bgez },
	{/* EBEQ */			            "BEQ",			default_handle_itype,   branch_issue_two_op,    handle_beq  },
	{/* EBNE */			            "BNE",			default_handle_itype,   branch_issue_two_op,    handle_bne  },
	{/* EBLEZ */			        "BLEZ",			default_handle_itype,   branch_issue_one_op,    handle_blez },
	{/* EBGTZ */			        "BGTZ",			default_handle_itype,   branch_issue_one_op,    handle_bgtz },
	{/* EADDI */			        "ADDI",			default_handle_itype,   itype_issue_two_op,     handle_addi },
	{/* EADDIU */			        "ADDIU",		default_handle_itype,   itype_issue_two_op,     handle_addiu},
	{/* ESLTI */			        "SLTI",			default_handle_itype,   itype_issue_two_op,     handle_slti },
	{/* ESLTIU */			        "SLTIU",		default_handle_itype,   itype_issue_two_op,     handle_sltiu},
	{/* ELW */			            "LW",			default_handle_itype,   load_issue,             handle_load },
	{/* ESW */			            "SW",			default_handle_itype,   store_issue,            handle_store}
};

void init_internal_mapping()
{
	memset(opcode_int_map, -2, sizeof(opcode_int_map));
	memset(opcode_rtype_map, -1, sizeof(opcode_rtype_map));

	opcode_int_map[0] = -1;
	opcode_int_map[1] = EBGEZ;
	opcode_int_map[2] = EJUMP;
	opcode_int_map[4] = EBEQ;
	opcode_int_map[5] = EBNE;
	opcode_int_map[6] = EBLEZ;
	opcode_int_map[7] = EBGTZ;
	opcode_int_map[8] = EADDI;
	opcode_int_map[9] = EADDIU;
	opcode_int_map[10] = ESLTI;
	opcode_int_map[11] = ESLTIU;
	opcode_int_map[35] = ELW;
	opcode_int_map[43] = ESW;

	opcode_rtype_map[0] = ESLL;
	opcode_rtype_map[2] = ESRL;
	opcode_rtype_map[3] = ESRA;
	opcode_rtype_map[13] = EBREAK;
	opcode_rtype_map[32] = EADD;
	opcode_rtype_map[33] = EADDU;
	opcode_rtype_map[34] = ESUB;
	opcode_rtype_map[35] = ESUBU;
	opcode_rtype_map[36] = EAND;
	opcode_rtype_map[37] = EOR;
	opcode_rtype_map[38] = EXOR;
	opcode_rtype_map[39] = ENOR;
	opcode_rtype_map[42] = ESLT;
	opcode_rtype_map[43] = ESLTU;
}

void display_ds(FILE* fout)
{
    int i=0, bcount=1, data_offset = (DATA_BASE_ADDR - CODE_BASE_ADDR)/sizeof(int);

    /* First print the clock cycle */
    fprintf(fout, "Cycle <%u>:\n", clk_cnt);

    /* Print IQ */
    fprintf(fout, "IQ:\n");
    for(i=gIQ.front; i != gIQ.rear; i = (i+1) % INSTR_Q_SIZE)
        fprintf(fout, "[%s]\n", gIQ.instr[i].instr_str);

    /* Print RS */
    fprintf(fout, "RS:\n");
    for(i=0; i < rs.count; i++)
        if(rob.rb[rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].rob_slot].state != ECOMMIT)
            fprintf(fout, "[%s]\n", rs.rs_item[(rs.front+i)%MAX_RS_ENTRIES].instr_string);

    /* Print ROB */
    fprintf(fout, "ROB:\n");
    for(i=0; i < rob.count; i++)
        if(rob.rb[(rob.front+i)%MAX_ROB_ENTRIES].state != ECOMMIT)
            fprintf(fout, "[%s]\n", rob.rb[(rob.front+i)%MAX_ROB_ENTRIES].instr_string);

    fprintf(fout, "BTB:\n");

    for(i=0; i < BTB_MAX_ENTRY; i++)
        if(branch_table[i].valid == true)
            fprintf(fout, "[Entry %d]<%u,%u,%d>\n", bcount++, branch_table[i].pc_addr, branch_table[i].target_addr, branch_table[i].prediction);

    fprintf(fout, "Registers:");

    for (i=0; i<GPR_MAX; i++) {
        if(i%8 == 0)
            fprintf(fout, "\nR%02d:", i);
        fprintf(fout, "\t%d", reg_file[i]);
    }

    fprintf(fout, "\nData Segment:\n716:");
    for(i = data_offset; i < (data_offset + 10); i++)
        fprintf(fout, "\t%d", mips_mem[i]);

    fprintf(fout, "\n");
}

void get_opt(const char* args[], int num_args, int *mptr, int *nptr)
{
    char *ptr;

    *mptr = -1; *nptr = -1;
    if((num_args < 3) || (num_args > 4)) {
        /* Invalid number of arguments. Displaye usage */
        printf("Incorrect number of arguments.. exiting..\n");
        printf("Usage: %s inputfilename outputfilename [-Tm:n]\n", args[0]);
        exit(1);
    }
    if(num_args == 4) {
        /* 4th Arguments may have start and end cycle */
        if('T' == getopt(num_args, args, "T:")) {
                ptr = optarg;
                *mptr = atoi(ptr);
                ptr = strchr(optarg, ':');
                *nptr = atoi(ptr+1);
        }
    }
    if((*mptr < 0) || (*nptr < 0)){
        *mptr = 1; *nptr = 500;
    }
}

void reset_ds()
{
    pc = rob.rb[rob.front].target_pc;

    gIQ.front = gIQ.rear;
    gIQ.count = 0;

    rs.front = rs.rear;
    rs.count = 0;

    rob.front = rob.rear;
    rob.count = 0;
}


int main (int argc, char *argv[])
{
	unsigned int i=0;
	unsigned int start_cycle, end_cycle;

	get_opt(argv, argc, &start_cycle, &end_cycle);

    FILE *fout = fopen(argv[2], "w");
    if (NULL == fout) {
        perror("Program encountered error exiting..\n");
        exit(1);
    }

	/* First load the memory with instructions */
	load_mips_mem(argv[1]);
	init_internal_mapping();
	gIQ.front = 0; gIQ.rear = 0; gIQ.count = 0; gIQ.stop_fetch = false;
	rob.front = 0; rob.rear = 0; rob.count = 0;
	rs.front = 0; rs.rear = 0; rs.count = 0;
	cdb.num_entries = 0;

	for (i=0; i < BTB_MAX_ENTRY; i++) {
        branch_table[i].prediction = -1;
        branch_table[i].valid = false;
    }

	for (i=0; i < GPR_MAX; i++)
        reg_status_table[i] = -1;
    bit_mask_arr[0] = 0xFFFFFFFF; //-1
    bit_mask_arr[1] = 0x7FFFFFFF;
    for (i=2; i<32; i++)
        bit_mask_arr[i] = bit_mask_arr[i-1] >> 1;

    do {
            reap_instruction();
			commit_instruction();
			writeback_instruction();
			mem_access();
			execute_instruction();
			issue_instruction();
			fetch_and_decode();

			if(flush_ds)
                reset_ds();

            /* Print all the DS here for machine state */
            if((clk_cnt >= start_cycle) && (clk_cnt <= end_cycle))
                display_ds(fout);
            clk_cnt++;
    }
    while(!end_simulation);

    /* Adjust the clock */
    clk_cnt--;

    if ((start_cycle == 0) && (end_cycle == 0))
        display_ds(fout);

    fclose(fout);

	return 0;
}
