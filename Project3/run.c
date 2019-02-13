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

instruction* get_inst_info(uint32_t pc);

    struct latch_update
{
    void* ptr;
    uint64_t value;
    uint32_t size;
};

static struct latch_update updatev[0x100];
static int updatec = 0;


void init_latch_update()
{
    updatec = 0;
}

void commit_latch_update()
{
    int i;
    for(i = 0; i < updatec; i++)
    {
        memcpy(updatev[i].ptr, &updatev[i].value, updatev[i].size);
    }
    updatec = 0;
}

void latch_update(void* ptr, uint64_t value, uint32_t size)
{
    updatev[updatec].ptr = ptr;
    updatev[updatec].value = value;
    updatev[updatec].size = size;
    updatec++;
}

void update_PC_Direct()
{
    int i;
    for(i = 0; i < updatec; i++)
    {
        if(updatev[i].ptr == &CURRENT_STATE.PC)
            memcpy(updatev[i].ptr, &updatev[i].value, updatev[i].size);
    }
    updatec = 0;

}

void IF_Stage()
{
    uint32_t is_stall = FALSE;
    uint32_t cpu_cand1 = CURRENT_STATE.PC + 4;
    latch_update(&CURRENT_STATE.IF_ID_NPC, cpu_cand1, sizeof(uint32_t));
    uint32_t cpu_cand2 = CURRENT_STATE.EX_MEM_BR_TARGET;
    //assert that ID stage is taken
    uint32_t cpu_cand3 = CURRENT_STATE.ID_JMP_ADDR;
    if(CURRENT_STATE.EX_MEM_BR_TAKE == TRUE)
    {
        latch_update(&CURRENT_STATE.EX_MEM_BR_TAKE, FALSE, sizeof(uint32_t));
        cpu_cand1 =  cpu_cand2;
    }
    if(CURRENT_STATE.ID_JMP_TAKE == TRUE)
    {
        cpu_cand1 = cpu_cand3;
        is_stall = TRUE;
        latch_update(&CURRENT_STATE.ID_JMP_TAKE, FALSE, sizeof(uint32_t));
    }
    if(!(CURRENT_STATE.ID_EX_MEM_READ == TRUE &&
        (CURRENT_STATE.ID_EX_RT == CURRENT_STATE.IF_ID_RS ||
        CURRENT_STATE.ID_EX_RT == CURRENT_STATE.IF_ID_RT)))
    {
        if(CURRENT_STATE.IF_STALL_PC == TRUE )
        {
            CURRENT_STATE.PIPE[IF_STAGE] = 0;
            CURRENT_STATE.IF_STALL_PC = FALSE;
            latch_update(&CURRENT_STATE.PC,cpu_cand1, sizeof(uint32_t));
            latch_update(&CURRENT_STATE.PIPE[ID_STAGE], 0, sizeof(uint32_t));
        }
        else if(is_stall == TRUE)
        {
            CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;
            CURRENT_STATE.IF_STALL_PC = FALSE;
            latch_update(&CURRENT_STATE.PC,cpu_cand1, sizeof(uint32_t));
            latch_update(&CURRENT_STATE.PIPE[ID_STAGE], 0, sizeof(uint32_t));

        }
        else
        {
	        if((CURRENT_STATE.PC < MEM_TEXT_START) || (CURRENT_STATE.PC >= MEM_TEXT_START + (NUM_INST << 2)))
	        {
                CURRENT_STATE.PIPE[IF_STAGE] = 0;
                latch_update(&CURRENT_STATE.PIPE[ID_STAGE], 0, sizeof(uint32_t));
	        }
            else
            {
                CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;
                latch_update(&CURRENT_STATE.PC,cpu_cand1, sizeof(uint32_t));
                latch_update(&CURRENT_STATE.PIPE[ID_STAGE], CURRENT_STATE.PC, sizeof(uint32_t));
            }

        }
            instruction* inst = get_inst_info(CURRENT_STATE.PC);
            latch_update(&CURRENT_STATE.IF_ID_OPCODE, OPCODE(inst), sizeof(OPCODE(inst)));
            latch_update(&CURRENT_STATE.IF_ID_FUNC, FUNC(inst), sizeof(FUNC(inst)));
            latch_update(&CURRENT_STATE.IF_ID_RS, RS(inst), sizeof(RS(inst)));
            latch_update(&CURRENT_STATE.IF_ID_RT, RT(inst), sizeof(RT(inst)));
            latch_update(&CURRENT_STATE.IF_ID_RD, RD(inst), sizeof(RD(inst)));
            latch_update(&CURRENT_STATE.IF_ID_SHAMT, SHAMT(inst), sizeof(SHAMT(inst)));
            latch_update(&CURRENT_STATE.IF_ID_IMM, IMM(inst), sizeof(IMM(inst)));
            latch_update(&CURRENT_STATE.IF_ID_TARGET, TARGET(inst), sizeof(TARGET(inst)));
    }
    else
    {
        CURRENT_STATE.PIPE[IF_STAGE] = CURRENT_STATE.PC;
    }

}

void ID_Stage()
{
    if(!CURRENT_STATE.PIPE[ID_STAGE] || CURRENT_STATE.PIPE_STALL[ID_STAGE] == TRUE)
    {
        latch_update(&CURRENT_STATE.PIPE[EX_STAGE], CURRENT_STATE.PIPE[ID_STAGE], sizeof(uint32_t));
        return;
    }

    // MUST called after WB
    if(CURRENT_STATE.IF_ID_OPCODE == 0x2)
    {
        CURRENT_STATE.ID_JMP_TAKE = TRUE;
        CURRENT_STATE.ID_JMP_ADDR = (CURRENT_STATE.IF_ID_TARGET << 2) | (CURRENT_STATE.ID_EX_NPC & 0xf0000000);
    }
    else if(CURRENT_STATE.IF_ID_OPCODE == 0x3)
    {
        CURRENT_STATE.ID_JMP_TAKE = TRUE;
        CURRENT_STATE.ID_JMP_ADDR = (CURRENT_STATE.IF_ID_TARGET << 2) | (CURRENT_STATE.ID_EX_NPC & 0xf0000000);
        latch_update(&CURRENT_STATE.ID_EX_REG1, CURRENT_STATE.IF_ID_NPC + 4, sizeof(uint32_t));
    }
    else if(CURRENT_STATE.IF_ID_OPCODE == 0x0 && CURRENT_STATE.IF_ID_FUNC == 0x8)
    {
        CURRENT_STATE.ID_JMP_TAKE = TRUE;
        CURRENT_STATE.ID_JMP_ADDR = CURRENT_STATE.REGS[CURRENT_STATE.IF_ID_RS];
    }
    else
    {
        latch_update(&CURRENT_STATE.ID_EX_REG1, CURRENT_STATE.REGS[CURRENT_STATE.IF_ID_RS & 0x1f] , sizeof(uint32_t));
        latch_update(&CURRENT_STATE.ID_EX_REG2, CURRENT_STATE.REGS[CURRENT_STATE.IF_ID_RT & 0x1f] , sizeof(uint32_t));
        latch_update(&CURRENT_STATE.ID_EX_R1, CURRENT_STATE.IF_ID_RS & 0x1f , sizeof(char));
        latch_update(&CURRENT_STATE.ID_EX_R2, CURRENT_STATE.IF_ID_RT & 0x1f , sizeof(char));
        latch_update(&CURRENT_STATE.ID_EX_RT, CURRENT_STATE.IF_ID_RT & 0x1f, sizeof(uint32_t));
        latch_update(&CURRENT_STATE.ID_EX_RD, CURRENT_STATE.IF_ID_RD & 0x1f, sizeof(uint32_t));
    }
    latch_update(&CURRENT_STATE.ID_EX_NPC, CURRENT_STATE.IF_ID_NPC, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.ID_EX_OPCODE, CURRENT_STATE.IF_ID_OPCODE, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.ID_EX_FUNC, CURRENT_STATE.IF_ID_FUNC, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.ID_EX_SHAMT, CURRENT_STATE.IF_ID_SHAMT, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.ID_EX_IMM, (int)CURRENT_STATE.IF_ID_IMM, sizeof(uint32_t));
    if(CURRENT_STATE.IF_ID_OPCODE == 0x23)
    {
        latch_update(&CURRENT_STATE.ID_EX_MEM_READ, TRUE, sizeof(uint32_t));
    }
    else
    {
        latch_update(&CURRENT_STATE.ID_EX_MEM_READ, FALSE, sizeof(uint32_t));
    }
    if(!(CURRENT_STATE.ID_EX_MEM_READ == TRUE &&
        (CURRENT_STATE.ID_EX_RT == CURRENT_STATE.IF_ID_RS ||
        CURRENT_STATE.ID_EX_RT == CURRENT_STATE.IF_ID_RT)))
    {
        latch_update(&CURRENT_STATE.PIPE[EX_STAGE], CURRENT_STATE.PIPE[ID_STAGE], sizeof(uint32_t));
    }
    else
    {
        latch_update(&CURRENT_STATE.PIPE[EX_STAGE], 0, sizeof(uint32_t));
    }

}
void EX_Stage()
{
    if(!CURRENT_STATE.PIPE[EX_STAGE])
    {
        latch_update(&CURRENT_STATE.EX_MEM_REG_WRITE, FALSE, sizeof(char));
        latch_update(&CURRENT_STATE.PIPE[MEM_STAGE], CURRENT_STATE.PIPE[EX_STAGE], sizeof(uint32_t));
        return;
    }
    uint32_t RREG1 = CURRENT_STATE.ID_EX_REG1;
    uint32_t RREG2 = CURRENT_STATE.ID_EX_REG2;
    if(CURRENT_STATE.MEM_WB_REG_WRITE == TRUE &&
        CURRENT_STATE.MEM_WB_FORWARD_REG != 0 &&
        CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_R1
        )
    {
        RREG1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
    }
    if(CURRENT_STATE.MEM_WB_REG_WRITE == TRUE &&
        CURRENT_STATE.MEM_WB_FORWARD_REG != 0 &&
        CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_R2
        )
        RREG2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;

    if(CURRENT_STATE.EX_MEM_REG_WRITE == TRUE &&
        CURRENT_STATE.EX_MEM_FORWARD_REG != 0 &&
        CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_R1
        )
        {
        RREG1 = CURRENT_STATE.EX_MEM_ALU_OUT;
        }

    if(CURRENT_STATE.EX_MEM_REG_WRITE == TRUE &&
        CURRENT_STATE.EX_MEM_FORWARD_REG != 0 &&
        CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_R2
        )
        RREG2 = CURRENT_STATE.EX_MEM_ALU_OUT;



    unsigned char r_dest = 0;
    unsigned char reg_write = FALSE;
    unsigned char mem_take = FALSE;
    unsigned char mem_read = FALSE;
    unsigned char mem_write = FALSE;
    uint32_t br_take = FALSE;
    uint32_t br_target = 0;
    switch(CURRENT_STATE.ID_EX_OPCODE)
    {
        //R type
        case 0x00:
        if(CURRENT_STATE.ID_EX_FUNC != 0x08)
        {
            reg_write = TRUE;
            r_dest = CURRENT_STATE.ID_EX_RD;
        }
        switch(CURRENT_STATE.ID_EX_FUNC)
        {
            case 0x08:
                break;
            case 0x21: //ADDU
                latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 + RREG2, sizeof(uint32_t));
                break;
            case 0x24: //AND
                latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 & RREG2, sizeof(uint32_t));
                break;
            case 0x27: //NOR
                latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, ~(RREG1 | RREG2), sizeof(uint32_t));
                break;
            case 0x25: //OR
                latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 | RREG2, sizeof(uint32_t));
                break;
            case 0x2b: //SLTU
                latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, (RREG1 < RREG2) ? 1 : 0, sizeof(uint32_t));
                break;
            case 0x00: //SLL
                latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG2 << CURRENT_STATE.ID_EX_SHAMT, sizeof(uint32_t));
                break;
            case 0x02: //SRL
                latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG2 >> CURRENT_STATE.ID_EX_SHAMT, sizeof(uint32_t));
                break;
            case 0x23: //SUBU
                latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 - RREG2, sizeof(uint32_t));
                break;
            default:
                printf("ERROR(NOT EXIST INST) R %d", CURRENT_STATE.ID_EX_FUNC);
        }
        break;
        //I type
        case 0x09: //ADDIU
            reg_write = TRUE;
            r_dest = CURRENT_STATE.ID_EX_RT;
            latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 + CURRENT_STATE.ID_EX_IMM, sizeof(uint32_t));
            break;
        case 0x0c: //ANDI
            reg_write = TRUE;
            r_dest = CURRENT_STATE.ID_EX_RT;
            latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 & CURRENT_STATE.ID_EX_IMM, sizeof(uint32_t));
            break;
        case 0x0f: //LUI
            reg_write = TRUE;
            r_dest = CURRENT_STATE.ID_EX_RT;
            latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, (CURRENT_STATE.ID_EX_IMM << 16), sizeof(uint32_t));
            break;
        case 0x23: //LW
            reg_write = TRUE;
            mem_read = TRUE;
            mem_take = TRUE;
            r_dest = CURRENT_STATE.ID_EX_RT;
            latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 + CURRENT_STATE.ID_EX_IMM, sizeof(uint32_t));
            break;
        case 0x0d: //ORI
            reg_write = TRUE;
            r_dest = CURRENT_STATE.ID_EX_RT;
            latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 | CURRENT_STATE.ID_EX_IMM, sizeof(uint32_t));
            break;
        case 0x0b: //SLTIU
            reg_write = TRUE;
            r_dest = CURRENT_STATE.ID_EX_RT;
            latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, (RREG1 < CURRENT_STATE.ID_EX_IMM) ? 1 : 0, sizeof(uint32_t));
            break;
        case 0x2b: //SW
            mem_write = TRUE;
            mem_take = TRUE;
            latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, RREG1 + CURRENT_STATE.ID_EX_IMM, sizeof(uint32_t));
            latch_update(&CURRENT_STATE.EX_MEM_W_VALUE, RREG2, sizeof(uint32_t));
            break;
        case 0x04: //BEQ
            if(RREG1 == RREG2)
            {
                br_take = TRUE;
                br_target = CURRENT_STATE.ID_EX_NPC + (CURRENT_STATE.ID_EX_IMM << 2);
            }
            break;
        case 0x05: //BNE
            if(RREG1 != RREG2)
            {
                br_take = TRUE;
                br_target = CURRENT_STATE.ID_EX_NPC + (CURRENT_STATE.ID_EX_IMM << 2);
            }
            break;

        //J type
        case 0x02:
            break;
        case 0x03: //JAL
            reg_write = TRUE;
            r_dest = 31;
            latch_update(&CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.ID_EX_REG1, sizeof(uint32_t));
            break;
        default:
        printf("ERROR(NOT EXIST INST) %d", CURRENT_STATE.ID_EX_OPCODE);
    }
    latch_update(&CURRENT_STATE.EX_MEM_FORWARD_REG, r_dest, sizeof(unsigned char));
    latch_update(&CURRENT_STATE.EX_MEM_DEST, r_dest, sizeof(unsigned char));
    latch_update(&CURRENT_STATE.EX_MEM_REG_WRITE, reg_write, sizeof(unsigned char));
    latch_update(&CURRENT_STATE.EX_MEM_MEM_TAKE, mem_take, sizeof(unsigned char));
    latch_update(&CURRENT_STATE.EX_MEM_MEM_READ, mem_read, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.EX_MEM_MEM_WRITE, mem_write, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.EX_MEM_BR_TAKE, br_take, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.EX_MEM_BR_TARGET, br_target, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.PIPE[MEM_STAGE], CURRENT_STATE.PIPE[EX_STAGE], sizeof(uint32_t));

}
void MEM_Stage()
{
    if(!CURRENT_STATE.PIPE[MEM_STAGE])
    {
        latch_update(&CURRENT_STATE.MEM_WB_REG_WRITE, FALSE, sizeof(char));
        latch_update(&CURRENT_STATE.PIPE[WB_STAGE], CURRENT_STATE.PIPE[MEM_STAGE], sizeof(uint32_t));
        return;
    }
    latch_update(&CURRENT_STATE.MEM_WB_FORWARD_REG, CURRENT_STATE.EX_MEM_FORWARD_REG, sizeof(unsigned char));

    if(CURRENT_STATE.EX_MEM_MEM_WRITE)
    {
        mem_write_32(CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.EX_MEM_W_VALUE);
    }
    if(CURRENT_STATE.EX_MEM_MEM_READ)
    {
        latch_update(&CURRENT_STATE.MEM_WB_MEM_OUT, mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT), sizeof(uint32_t));
        latch_update(&CURRENT_STATE.MEM_WB_FORWARD_VALUE, mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT), sizeof(uint32_t));
    }
    else
    {
        latch_update(&CURRENT_STATE.MEM_WB_FORWARD_VALUE, CURRENT_STATE.EX_MEM_ALU_OUT, sizeof(uint32_t));
    }
    latch_update(&CURRENT_STATE.MEM_WB_DEST, CURRENT_STATE.EX_MEM_DEST, sizeof(unsigned char));
    latch_update(&CURRENT_STATE.MEM_WB_REG_WRITE, CURRENT_STATE.EX_MEM_REG_WRITE, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.MEM_WB_ALU_OUT, CURRENT_STATE.EX_MEM_ALU_OUT, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.MEM_WB_MEM_TAKE, CURRENT_STATE.EX_MEM_MEM_TAKE, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.MEM_WB_MEM_READ, CURRENT_STATE.EX_MEM_MEM_READ, sizeof(uint32_t));
    latch_update(&CURRENT_STATE.PIPE[WB_STAGE], CURRENT_STATE.PIPE[MEM_STAGE], sizeof(uint32_t));
}
void WB_Stage()
{
    if(!CURRENT_STATE.PIPE[WB_STAGE])
        return;
    uint32_t write_val;
    if(CURRENT_STATE.MEM_WB_MEM_TAKE)
    {
        write_val = CURRENT_STATE.MEM_WB_MEM_OUT;
    }
    else
    {
        write_val = CURRENT_STATE.MEM_WB_ALU_OUT;
    }
    if(CURRENT_STATE.MEM_WB_REG_WRITE)
    {
        CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = write_val;
    }
    INSTRUCTION_COUNT++;
}

void check_branch_stall()
{
    if(CURRENT_STATE.EX_MEM_BR_TAKE == TRUE)
    {
        CURRENT_STATE.PIPE[IF_STAGE] = 0;
        CURRENT_STATE.IF_STALL_PC = TRUE;
        CURRENT_STATE.PIPE[ID_STAGE] = 0;
        CURRENT_STATE.PIPE[EX_STAGE] = 0;
    }
}
/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) {
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

void final_fixup()
{
    update_PC_Direct();
}


/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
    commit_latch_update();
    check_branch_stall();
    uint32_t IS_STOP = TRUE;
    int i;
    for(i = 0; i < 4; i++)
    {
        if(CURRENT_STATE.PIPE[i] != 0)
        {
            IS_STOP = FALSE;
            break;
        }
    }

    init_latch_update();
    WB_Stage();
    ID_Stage();
    IF_Stage();
    MEM_Stage();
    EX_Stage();
	if(((CURRENT_STATE.PC < MEM_TEXT_START) || (CURRENT_STATE.PC >= MEM_TEXT_START + (NUM_INST << 2))) && IS_STOP == TRUE)
	{
		RUN_BIT = FALSE;
		return;
	}

    if(INSTRUCTION_COUNT >= MAX_INSTRUCTION_NUM)
    {
        final_fixup();
    }
}
