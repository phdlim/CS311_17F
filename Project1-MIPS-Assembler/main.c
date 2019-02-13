#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "mips_parser.h"
#include "mips_generator.h"
#include "mips_text.h"
#include "mips_util.h"

void process_data(struct mips_parser* parser, struct mips_generator* generator)
{
    char** d_argv = parser->argv;
    int d_argc = parser->argc;
    if(d_argc > 0 && strlen(d_argv[0]) >= 1 && d_argv[0][strlen(d_argv[0])-1] == ':')
    {
        d_argv[0][strlen(d_argv[0])-1] = '\x00';
        mips_generator_register_data_label(generator, d_argv[0]);
        d_argc--;
        d_argv = &d_argv[1];
    }
    //assert(argc == 2 && !strcmp(d_argv[0], ".word"));
    mips_generator_insert_data(generator, imm_convert(d_argv[1]));


}

void process_text(struct mips_parser* parser, struct mips_generator* generator)
{
    char** t_argv = parser->argv;
    int t_argc = parser->argc;
    if(t_argc > 0 && strlen(t_argv[0]) >= 1 && t_argv[0][strlen(t_argv[0])-1] == ':')
    {
        t_argv[0][strlen(t_argv[0])-1] = '\x00';
        mips_generator_register_text_label(generator, t_argv[0]);
        t_argc--;
        t_argv = &t_argv[1];
    }

    struct type
    {
    char *funcname;
    char typename;
    };

    struct type function[21] = {{"addiu", 'I'},
                {"addu" , 'R'},
                {"and"  , 'R'},
                {"andi" , 'I'},
                {"beq"  , 'I'},
                {"bne"  , 'I'},
                {"j"    , 'J'},
                {"jal"  , 'J'},
                {"jr"   , 'R'},
                {"la"   , 'P'},    //pseudo function
                {"lui"  , 'I'},
                {"lw"   , 'I'},
                {"or"   , 'R'},
                {"nor"  , 'R'},
                {"ori"  , 'I'},
                {"sltiu", 'I'},
                {"sltu" , 'R'},
                {"sll"  , 'R'},
                {"srl"  , 'R'},
                {"sw"   , 'I'},
                {"subu" , 'R'}};
    char format = 0;
    int i = 0;

    if(t_argc >= 1)
    {
        //printf("operation : %s\n", t_argv[0]);
        for(i = 0; i < 21; i++)
        {
            if(!strcmp(t_argv[0], function[i].funcname))
            format = function[i].typename;
        }

        switch(format)
        {
            case 'R':
                type_R(generator, t_argc, t_argv);
                break;
            case 'I':
                type_I(generator, t_argc, t_argv);
                break;
            case 'J':
                type_J(generator, t_argc, t_argv);
                break;
            case 'P':
                type_pseudo(generator, t_argc, t_argv);
                break;
            default:
                puts("Error: Not Exist Function");
                break;
        }
    }
}

int main(int argc, char* argv[])
{

    FILE *input, *output;
    const char* in_filename;
    char *out_filename;

    if(argc < 2)
    {
        puts("Error : No Input File");
        return 0;
    }
    else if(argc == 2)
    {
        in_filename = argv[1];
        out_filename = (char*)malloc(strlen(in_filename) + 3);
        strcpy(out_filename, in_filename);
        int i, conv = 0;
        for(i = 0; i < strlen(out_filename); i++)
        {
            if(out_filename[strlen(out_filename) - 1 - i] == '/')
            {
                break;
            }
            if(out_filename[strlen(out_filename) - 1 - i] == '.')
            {
                int to_fix = strlen(out_filename) - 1 - i;
                out_filename[to_fix] = '.';
                out_filename[to_fix+1] = 'o';
                out_filename[to_fix+2] = '\0';
                conv = 1;
                break;
            }
        }
        if(!conv)
        {
            int to_fix = strlen(out_filename);
            out_filename[to_fix] = '.';
            out_filename[to_fix+1] = 'o';
            out_filename[to_fix+2] = '\0';
            conv = 1;

        }
        input = fopen(in_filename, "r");
        if(!input)
        {
            puts("Error : Failed to open file");
            return 1;
        }
    }
    else
    {
        puts("Error : Two or More Input Files");
        return 0;
    }

    int data=0, text=0;
    struct mips_parser* parser = new_mips_parser(input);
    struct mips_generator* generator = new_mips_generator();
    while(mips_parser_next_line(parser))
    {
        // to debug
        // mips_parser_print_current(parser);
        if(parser->argc == 1
                 && strlen(parser->argv[0]) == 5
                 && !strncmp(parser->argv[0], ".data", 5))
        {
            data=1;
            //puts("found data");
        }
        else if(data==1)
        {
            if(parser->argc == 1
                    && strlen(parser->argv[0]) == 5
                    && !strncmp(parser->argv[0], ".text", 5))
            {
                text=1;
                data=0;
                //puts("found text");
            }
            else
            {
                process_data(parser, generator);
            }
        }
        else if(text==1)
        {
            process_text(parser, generator);
        }
    }
    mips_generator_commit_all(generator);
    mips_generator_emit_code(generator, out_filename);
    del_mips_parser(parser);
    del_mips_generator(generator);
    return 0;
}


