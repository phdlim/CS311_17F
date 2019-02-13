/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

int text_size;
int data_size;

instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;
    int i;
	/** Implement this function */
    memset(&instr,'\x00', sizeof(instruction));
    for(i = 0; i < 6; i++)
    {
        instr.opcode <<= 1;
        instr.opcode |= (buffer[i] & 1);
        instr.value <<= 1;
        instr.value |= (buffer[i] & 1);
    }
    switch(instr.opcode)
    {
        //TYPE R
        case 0x0:        //(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
            for(; i < 11; i++)
            {
                instr.r_t.r_i.rs <<= 1;
                instr.r_t.r_i.rs |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }
            for(; i < 16; i++)
            {
                instr.r_t.r_i.rt <<= 1;
                instr.r_t.r_i.rt |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }
            for(; i < 21; i++)
            {
                instr.r_t.r_i.r_i.r.rd <<= 1;
                instr.r_t.r_i.r_i.r.rd |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }
            for(; i < 26; i++)
            {
                instr.r_t.r_i.r_i.r.shamt <<= 1;
                instr.r_t.r_i.r_i.r.shamt |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }
            for(; i < 32; i++)
            {
                instr.func_code <<= 1;
                instr.func_code |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }
            break;
        //TYPE J
        case 0x2:       //(0x000010)J
        case 0x3:       //(0x000011)JAL
            for(; i < 32; i++)
            {
                instr.r_t.target <<= 1;
                instr.r_t.target |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }
            break;
        //Type I
        case 0x9:        //(0x001001)ADDIU
        case 0xc:        //(0x001100)ANDI
        case 0xf:        //(0x001111)LUI
        case 0xd:        //(0x001101)ORI
        case 0xb:        //(0x001011)SLTIU
        case 0x23:        //(0x100011)LW
        case 0x2b:        //(0x101011)SW
        case 0x4:        //(0x000100)BEQ
        case 0x5:        //(0x000101)BNE
            for(; i < 11; i++)
            {
                instr.r_t.r_i.rs <<= 1;
                instr.r_t.r_i.rs |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }
            for(; i < 16; i++)
            {
                instr.r_t.r_i.rt <<= 1;
                instr.r_t.r_i.rt |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }
            for(; i < 32; i++)
            {
                instr.r_t.r_i.r_i.imm <<= 1;
                instr.r_t.r_i.r_i.imm |= (buffer[i] & 1);
                instr.value <<= 1;
                instr.value |= (buffer[i] & 1);
            }

            break;
        default:
            puts("Not available instruction");
            assert(0);
    }
    mem_write_32(MEM_TEXT_START + index, instr.value);
    return instr;
}

void parsing_data(const char *buffer, const int index)
{
	/** Implement this function */
    int i;
    unsigned int val = 0;
    for(i = 0; i < 32; i ++)
    {
        val <<= 1;
        val |= (buffer[i] & 1);
    }
    mem_write_32(MEM_DATA_START + index, val);

}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
	printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
	printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

	switch(INST_INFO[i].opcode)
	{
	    //Type I
	    case 0x9:		//(0x001001)ADDIU
	    case 0xc:		//(0x001100)ANDI
	    case 0xf:		//(0x001111)LUI	
	    case 0xd:		//(0x001101)ORI
	    case 0xb:		//(0x001011)SLTIU
	    case 0x23:		//(0x100011)LW	
	    case 0x2b:		//(0x101011)SW
	    case 0x4:		//(0x000100)BEQ
	    case 0x5:		//(0x000101)BNE
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
		break;

    	    //TYPE R
	    case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
		printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
		printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
		break;

    	    //TYPE J
	    case 0x2:		//(0x000010)J
	    case 0x3:		//(0x000011)JAL
		printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
		break;

	    default:
		printf("Not available instruction\n");
		assert(0);
	}
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
	printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
	printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
