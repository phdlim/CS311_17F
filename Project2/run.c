/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/

void process_instruction(){
	/** Implement this function */
	instruction* inst;
	inst = get_inst_info(CURRENT_STATE.PC);
	
	if((CURRENT_STATE.PC < MEM_TEXT_START) || (CURRENT_STATE.PC >= MEM_TEXT_START + (NUM_INST << 2)))
	{
		RUN_BIT = FALSE;
		return;
	}

	CURRENT_STATE.PC += 4;

	switch(OPCODE(inst))
	{
	// R TYPE
	case 0x00:
		switch(FUNC(inst))
		{
			case 0x21: //ADDU
				CURRENT_STATE.REGS[RD(inst)] = CURRENT_STATE.REGS[RS(inst)] + CURRENT_STATE.REGS[RT(inst)];
				break;
			case 0x24: //AND
				CURRENT_STATE.REGS[RD(inst)] = CURRENT_STATE.REGS[RS(inst)] & CURRENT_STATE.REGS[RT(inst)];
				break;
			case 0x08:  //JR
				CURRENT_STATE.PC = CURRENT_STATE.REGS[RS(inst)];
				break;
			case 0x27: //NOR
				CURRENT_STATE.REGS[RD(inst)] = ~(CURRENT_STATE.REGS[RS(inst)] | CURRENT_STATE.REGS[RT(inst)]);
				break;
			case 0x25: //OR
				CURRENT_STATE.REGS[RD(inst)] = CURRENT_STATE.REGS[RS(inst)] | CURRENT_STATE.REGS[RT(inst)];
				break;
			case 0x2b: //SLTU
				CURRENT_STATE.REGS[RD(inst)] = (CURRENT_STATE.REGS[RS(inst)] < CURRENT_STATE.REGS[RT(inst)]) ? 1 : 0;
				break;
			case 0x00: //SLL
				CURRENT_STATE.REGS[RD(inst)] = CURRENT_STATE.REGS[RT(inst)] << SHAMT(inst);
				break;
			case 0x02: //SRL
				CURRENT_STATE.REGS[RD(inst)] = CURRENT_STATE.REGS[RT(inst)] >> SHAMT(inst);
				break;
			case 0x23: //SUBU
				CURRENT_STATE.REGS[RD(inst)] = CURRENT_STATE.REGS[RS(inst)] - CURRENT_STATE.REGS[RT(inst)];
				break;
			default:
				printf("ERROR(NOT EXIST INST)");

		}
		break;
	//I type
	case 0x09: //ADDIU
		CURRENT_STATE.REGS[RT(inst)] = CURRENT_STATE.REGS[RS(inst)] + SIGN_EX(IMM(inst));
		break;
	case 0x0c: //ANDI
		CURRENT_STATE.REGS[RT(inst)] = CURRENT_STATE.REGS[RS(inst)] & IMM(inst);
		break;
	case 0x04: //BEQ
		BRANCH_INST(CURRENT_STATE.REGS[RS(inst)] == CURRENT_STATE.REGS[RT(inst)], (SIGN_EX(IMM(inst)) << 2) + CURRENT_STATE.PC, NULL);
		break;
	case 0x05: //BNE
		BRANCH_INST(CURRENT_STATE.REGS[RS(inst)] != CURRENT_STATE.REGS[RT(inst)], (SIGN_EX(IMM(inst)) << 2) + CURRENT_STATE.PC, NULL);
		break;
	case 0x0f: //LUI
		CURRENT_STATE.REGS[RT(inst)] = (IMM(inst) << 16);
		break;
	case 0x23: //LW
		CURRENT_STATE.REGS[RT(inst)] = mem_read_32(CURRENT_STATE.REGS[RS(inst)] + SIGN_EX(IMM(inst)));
		break;
	case 0x0d: //ORI
		CURRENT_STATE.REGS[RT(inst)] = CURRENT_STATE.REGS[RS(inst)] | IMM(inst);
		break;
	case 0x0b: //SLTIU
		CURRENT_STATE.REGS[RT(inst)] = (CURRENT_STATE.REGS[RS(inst)] < (IMM(inst))) ? 1 : 0;
		break;
	case 0x2b: //SW
		mem_write_32(CURRENT_STATE.REGS[RS(inst)]+SIGN_EX(IMM(inst)), CURRENT_STATE.REGS[RT(inst)]);
		break;
	
	//J type
	case 0x02: //J
		CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) | (TARGET(inst) << 2);
		break;
	case 0x03: //JAL
		CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
		CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) | (TARGET(inst) << 2);
		break;
	default:
		printf("ERROR(NOT EXIST INST)");
	}
}

