#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "mips_parser.h"
#include "mips_generator.h"
#include "mips_util.h"

void type_R(struct mips_generator* generator, int t_argc, char** t_argv)
{
    char* result= (char*)malloc(32+1);
    char op[7];
    char rs[6];
    char rt[6];
    char rd[6];
    char shamt[6];
    char funct[7];

    struct R
    {
        char *op_name;
        char *op;
        char *funct;
    };

    struct R function[9] = {{"addu", "000000", "100001"},
                {"and" , "000000", "100100"},
                {"jr"  , "000000", "001000"},
                {"nor" , "000000", "100111"},
                {"or"  , "000000", "100101"},
                {"sltu", "000000", "101011"},
                {"sll" , "000000", "000000"},
                {"srl" , "000000", "000010"},
                {"subu", "000000", "100011"}};

    int number = -1;
    int i = 0;

    for(i = 0; i < 9; i++)
    {
        if(!strcmp(function[i].op_name, t_argv[0]))
        {
            number = i;
            break;
        }
    }
    switch(number)
    {
    case 2:
        strncpy( op, function[number].op, 7);
        strncpy( rs, convert2_5bit(register_convert(t_argv[1])), 6);
        strncpy( rt, "00000", 6);
        strncpy( rd, "00000", 6);
        strncpy( shamt, "00000", 6);
        strncpy( funct, function[number].funct, 7);
            break;
    case 6:
    case 7:
        strncpy( op, function[number].op, 7);
        strncpy( rs, "00000", 6);
        strncpy( rt, convert2_5bit(register_convert(t_argv[2])), 6);
        strncpy( rd, convert2_5bit(register_convert(t_argv[1])), 6);
        strncpy( shamt, convert2_5bit(atoi(t_argv[3])), 6);
        strncpy( funct, function[number].funct, 7);
        break;
    case 0:
    case 1:
    case 3:
    case 4:
    case 5:
    case 8:
        strncpy( op, function[number].op, 7);
        strncpy( rs, convert2_5bit(register_convert(t_argv[2])), 6);
        strncpy( rt, convert2_5bit(register_convert(t_argv[3])), 6);
        strncpy( rd, convert2_5bit(register_convert(t_argv[1])), 6);
        strncpy( shamt, "00000", 6);
        strncpy( funct, function[number].funct, 7);
        break;
    default:
        puts("Error : Cannot Convert to Binary(Function Type R)");
        exit(0);
    }

    for(i = 0; i < 32; i++)
    {
        if(i >= 0 && i < 6)
            result[i] = op[i];
        else if(i >= 6 && i < 11)
            result[i] = rs[i - 6];
        else if(i >= 11 && i < 16)
            result[i] = rt[i - 11];
        else if(i >= 16 && i < 21)
            result[i] = rd[i - 16];
        else if(i >= 21 && i < 26)
            result[i] = shamt[i - 21];
        else
            result[i] = funct[i - 26];
    }
    if(strlen(result) < 32)
    {
        puts("Error : Not complete Binary");
        exit(0);
    }
    //printf("%s\n", result);
    mips_generator_insert_text(generator, result);
}

void type_I(struct mips_generator* generator, int t_argc, char** t_argv)
{
    char* result = (char*)malloc(32+1);
    char op[7];
    char rs[6];
    char rt[6];
    char imm[17];

    struct I
    {
        char *op_name;
        char *op;
    };

    struct I function[9] = {{"addiu", "001001"},
                {"andi" , "001100"},
                {"beq"  , "000100"},
                {"bne"  , "000101"},
                {"lui"  , "001111"},
                {"lw"   , "100011"},
                {"ori"  , "001101"},
                {"sltiu", "001011"},
                {"sw"   , "101011"}};   

    int number = -1;
    int i = 0;
    int uselabel = 0;
    for(i = 0; i < 9; i++)
    {
        if(!strcmp(function[i].op_name, t_argv[0]))
        {
            number = i;
            break;
        }
    }

    switch(number)
    {
        case 0:
        case 1:
        case 6:
        case 7:
            strncpy( op, function[number].op, 7);
            strncpy( rs, convert2_5bit(register_convert(t_argv[2])), 6);
            strncpy( rt, convert2_5bit(register_convert(t_argv[1])), 6);
            strncpy( imm, convert2_16bit(imm_convert(t_argv[3])), 17);
            break;
        case 2:
        case 3:  //need label
            strncpy( op, function[number].op, 7);
            strncpy( rs, convert2_5bit(register_convert(t_argv[1])), 6);
            strncpy( rt, convert2_5bit(register_convert(t_argv[2])), 6);
            memset( imm, '0', 16);
	        imm[16] = '\x00';
	        uselabel = 1;
        //mips_generator_promise_text(generator, result, t_argv[3], 16, 31);
            break;
        case 4:
            strncpy( op, function[number].op, 7);
            strncpy( rs, "00000", 6);
            strncpy( rt, convert2_5bit(register_convert(t_argv[1])), 6);
            strncpy( imm, convert2_16bit(imm_convert(t_argv[2])), 17);
            break;
        case 5:
        case 8:
            strncpy( op, function[number].op, 7);
	    strncpy( rs, convert2_5bit(register_convert(t_argv[3])), 6);
	    strncpy( rt, convert2_5bit(register_convert(t_argv[1])), 6);
	    strncpy( imm, convert2_16bit(imm_convert(t_argv[2])), 16);
            break;
        default:
            puts("Error : Cannot Convert to Binary(Function Type I)");
            exit(0);
    }
    for(i = 0; i < 32; i++)
    {
        if(i >= 0 && i < 6)
            result[i] = op[i];
        else if(i >= 6 && i < 11)
            result[i] = rs[i - 6];
        else if(i >= 11 && i < 16)
            result[i] = rt[i - 11];
        else
            result[i] = imm[i - 16];
    }
    if(strlen(result) < 32)
    {
        puts("Error : Not Complete Binary");
    }
    //printf("%s\n", result);
    if(uselabel)
    {
        mips_generator_promise_text(generator, result, t_argv[3], 16, 31);
    }
    else
    {
        mips_generator_insert_text(generator, result);
    }
}

void type_J(struct mips_generator* generator, int t_argc, char** t_argv)   //need label in this function
{
    char* result = (char*)malloc(32+1);
    char op[7];
    char add[27];

    struct J
    {
        char *op_name;
        char *op;
    };
    
    struct J function[2] = {{"j"  , "000010"},
                {"jal", "000011"}};

    int number = -1;
    int i = 0;

    for(i = 0; i < 2; i++)
    {
        if(!strcmp(function[i].op_name, t_argv[0]))
        {
            number = i;
            break;
        }
    }
    
    switch(number)
    {
        case 0:
        case 1:
            strncpy( op, function[number].op, 7);
            memset(add, '0', 26);
            add[26] = '\x00';
            break;
        default:
            puts("Error : Cannot Convert to Binary(Function Type J)");
    }
    for(i = 0; i < 32; i++)
    {
        if(i >= 0 && i < 6)
            result[i] = op[i];
        else
            result[i] = add[i - 6];
    }
    if(strlen(result) <32)
    {
                puts("Error : Not Complete Binary");
        }
    //printf("%s\n", result);
    mips_generator_promise_text(generator, result, t_argv[1], 6, 31);
}

void type_pseudo(struct mips_generator* generator, int t_argc, char** t_argv)
{
    assert(t_argc == 3);
    unsigned int target = 0;
    char *fake_arg[4];
    unsigned char tmpbuf[0x10];
    if( '0' <= t_argv[2][0] && t_argv[2][0] <= '9' )
    {
        target = imm_convert(t_argv[2]);
    }
    else
    {
        target = mips_generator_get_label(generator, t_argv[2]);
    }
    //printf("%x\n", target);
    snprintf(tmpbuf, 0x10, "0x%x", target >> 16);
    fake_arg[0] = "lui";
    fake_arg[1] = t_argv[1];
    fake_arg[2] = tmpbuf;
    type_I(generator, 3, fake_arg);
    if(target & 0xffff)
    {
        snprintf(tmpbuf, 0x10, "0x%x", target & 0xffff);
        fake_arg[0] = "ori";
        fake_arg[1] = t_argv[1];
        fake_arg[2] = t_argv[1];
        fake_arg[3] = tmpbuf;
        type_I(generator, 4, fake_arg);
    }
}
