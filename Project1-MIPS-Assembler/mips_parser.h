#ifndef __MIPS_PARSER_H__
#define __MIPS_PARSER_H__

#include <stdio.h>

struct mips_parser
{
    FILE* in;
    int tok_start;
    int tok_end;
    char* tok_p;
    char** argv;
    int argc;
    char linebuf[0x100];
};

struct mips_parser* new_mips_parser(FILE* in);

int mips_parser_next_line(struct mips_parser*);

char* mips_parser_next_token(struct mips_parser*);

void del_mips_parser(struct mips_parser*);

void mips_parser_print_current(struct mips_parser*);
#endif
