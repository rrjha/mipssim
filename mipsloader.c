#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "mipsloader.h"

#define CNV_ENDIAN(num) \
		(((num>>24)&0x000000FF) | \
                ((num>>8)&0x0000FF00)   | \
		((num<<8)&0x00FF0000)   | \
		((num<<24)&0xFF000000));

/* Abstract memory as words from 600 to 800 */
int32 mips_mem[MEM_SIZE];
int reg_file[GPR_MAX];

/* Read the given input file and load the instructions in simulated memory */
void load_mips_mem (const char *input_mips_file)
{
	FILE *fin;
	int filesz=0, testval = 1;
	int wordsread = 0;

	/* Init the mem first */
	memset(mips_mem, 0, sizeof(mips_mem));
	memset(reg_file, 0, sizeof(reg_file));

	fin = fopen (input_mips_file, "rb");
    if (NULL == fin) {
        perror("Program encountered error exiting..\n");
        exit(1);
    }
	fseek(fin, 0L, SEEK_END);
	filesz = ftell (fin);
	if ((filesz <= 0) || ((filesz % 4) != 0)) {
		printf("The size of input file %s is not multiple of 4 bytes. MIPS instruction must be exactly 32 bit.\n", input_mips_file);
		fclose(fin);
		exit(1);
	}
	rewind(fin);

	while ((sizeof(int32)*wordsread < filesz) && (wordsread < MEM_SIZE)){
		fread(&(mips_mem[wordsread]), sizeof(int32), 1, fin);
		if (*(char*)&testval)
			mips_mem[wordsread] = CNV_ENDIAN(mips_mem[wordsread]);
		wordsread ++;
	}

	fclose(fin);
}
