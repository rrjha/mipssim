#ifndef DATATYPES_H
#define DATATYPES_H

#define MAX_RS_ENTRIES 10
#define MAX_ROB_ENTRIES 6
#define GPR_MAX 32
#define INSTR_Q_SIZE 1024
#define BTB_MAX_ENTRY 16

typedef unsigned char uint8;
typedef int int32;
typedef unsigned int uint32;
typedef enum RobState RobState;

typedef struct jformat_instr JFormat_Instr;
typedef struct iformat_instr IFormat_Instr;
typedef struct rformat_instr RFormat_Instr;
typedef struct instr_q_entry instr_q_entry;
typedef struct reservation_station_entry rs_entry;
typedef struct reorder_buffer_entry rob_entry;

typedef void (*fetch_decode_handle)(const unsigned int);
typedef void (*rs_issue_handle)(const instr_q_entry*, rs_entry*);
typedef void (*execute_handle)(rs_entry*);


/* List of supported instructions are discontinuous so keep a enumerated list */
/* Internal mapping */
typedef enum instr_int_code{
	/* Rtype Instructions with opcode 0 */
	EINSTRUCTION_BEGIN = 0,
	ENOP = EINSTRUCTION_BEGIN,	/* funct = 0, sa = 0	*/
	ESLL,				/* funct = 0, sa != 0	*/
	ESRL,				/* funct = 2		*/
	ESRA,				/* funct = 3		*/
	EBREAK,				/* funct = 13		*/
	EADD,				/* funct = 32		*/
	EADDU,				/* funct = 33		*/
	ESUB,				/* funct = 34		*/
	ESUBU,				/* funct = 35		*/
	EAND,				/* funct = 36		*/
	EOR,				/* funct = 37		*/
	EXOR,				/* funct = 38		*/
	ENOR,				/* funct = 39		*/
	ESLT,				/* funct = 42		*/
	ESLTU,				/* funct = 43		*/
	ERTYPE_MAX = ESLTU,
	EJUMP,				/* opcode=2	*/
	EJTYPE_MAX=EJUMP,
	EBLTZ,				/* opcode=1, rt=0	*/
	EBGEZ,				/* opcode=1, rt=1	*/
	EBEQ,				/* opcode=4		*/
	EBNE,				/* opcode=5		*/
	EBLEZ,				/* opcode=6		*/
	EBGTZ,				/* opcode=7		*/
	EADDI,				/* opcode=8		*/
	EADDIU,				/* opcode=9		*/
	ESLTI,				/* opcode=10		*/
	ESLTIU,				/* opcode=11		*/
	ELW,				/* opcode=35		*/
	ESW,				/* opcode=43		*/
	EITYPE_MAX=ESW,
	EINSTRUCTION_MAX
} INSTRCODE;

#endif
