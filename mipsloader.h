#ifndef MIPSLOADER_H
#define MIPSLOADER_H
#include "mipssim.h"

#define CODE_BASE_ADDR 600
#define DATA_BASE_ADDR 716
#define MEM_END 800
#define MEM_SIZE ((MEM_END - CODE_BASE_ADDR)/sizeof(int32))

//#define INSTR_MEM_SIZE (DATA_BASE_ADDR - CODE_BASE_ADDR)/sizeof(uint32)
//#define DATA_MEM_SIZE 16	/* Since the problem doesn't give one so assume some reasonable data mem size */

void load_mips_mem (const char*);

extern int32 mips_mem[MEM_SIZE];
extern int reg_file[GPR_MAX];

#endif
