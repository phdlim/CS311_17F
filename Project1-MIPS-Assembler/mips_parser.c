#include "mips_parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct mips_parser* new_mips_parser(FILE* in)
{
    struct mips_parser* retval = (struct mips_parser*)malloc(sizeof (struct mips_parser));
    retval->in = in;
    retval->tok_start = 0;
    retval->tok_end = 0;
    retval->tok_p = 0;
    retval->argv = 0;
    retval->argc = 0;
    memset(retval->linebuf, '\x00', sizeof(retval->linebuf));
    return retval;
}

int mips_parser_next_line(struct mips_parser* this)
{
    int argc = 0;
    int i;
    this->tok_start = 0;
    this->tok_end = 0;
    if(this->argv)
    {
        for(i = 0; i < this->argc; i++)
        {
            free(this->argv[i]);
        }
        free(this->argv);
        this->argv = 0;
    }
    if(mips_parser_has_next(this))
    {
        fgets(this->linebuf, 0x100, this->in);
        if(feof(this->in)) return 0;
        while (mips_parser_next_token_int(this)) argc++;
        this->argc = argc;
        this->tok_start = 0;
        this->tok_end = 0;
        this->argv = (char**)malloc(sizeof(char*) * argc);
        for(i = 0; i < argc; i ++)
        {
            this->argv[i] = mips_parser_next_token(this);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

int mips_parser_filter(char s)
{
    return isspace(s) || s == ',' || s == '(' || s == ')';
}

int mips_parser_next_token_int(struct mips_parser* this)
{
    // find first position which is not space
    while(mips_parser_filter(this->linebuf[this->tok_start]) && this->tok_start < strlen(this->linebuf)) this->tok_start++;
    // if all string is space ( empty line ), ignore
    if(this->tok_start >= strlen(this->linebuf)) return 0;

    this->tok_end = this->tok_start;
    // find end of token
    while(!mips_parser_filter(this->linebuf[this->tok_end]) && this->tok_end < strlen(this->linebuf)) this->tok_end++;
    this->tok_start = this->tok_end;
    return 1;
}


char* mips_parser_next_token(struct mips_parser* this)
{

    // find first position which is not space
    while(mips_parser_filter(this->linebuf[this->tok_start]) && this->tok_start < strlen(this->linebuf)) this->tok_start++;
    // if all string is space ( empty line ), ignore
    if(this->tok_start >= strlen(this->linebuf)) return NULL;

    this->tok_end = this->tok_start;
    // find end of token
    while(!mips_parser_filter(this->linebuf[this->tok_end]) && this->tok_end < strlen(this->linebuf)) this->tok_end++;
    this->tok_p = (char*)malloc(this->tok_end - this->tok_start + 1);
    memcpy(this->tok_p, &this->linebuf[this->tok_start], this->tok_end - this->tok_start);
    this->tok_p[this->tok_end - this->tok_start] = '\x00';
    this->tok_start = this->tok_end;
    return this->tok_p;

}



int mips_parser_has_next(struct mips_parser* this)
{
    return !feof(this->in);
}

int mips_parser_token_len(struct mips_parser* this)
{
    return this->tok_end - this->tok_start;
}

void del_mips_parser(struct mips_parser* this)
{
    fclose(this->in);
}

void mips_parser_print_current(struct mips_parser* this)
{
    int i;
    printf("{ ");
    for(i = 0; i + 1< this->argc; i++)
    {
        printf("\"%s\", ", this->argv[i]);
    }
    if(this->argc > 0)
    {
        printf("\"%s\" }\n", this->argv[this->argc-1]);
    }
    else
    {
        printf("}\n");
    }
}
