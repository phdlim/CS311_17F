#include <stdlib.h>
#include <string.h>
#include "mips_util.h"

char* convert2_5bit(int a)
{
    int i = 0;
    int bit;
    char* b = (char*)malloc(5+1);
    for(i = 0; i < 5; i++)
    {
        bit = a & (1<<(4-i));
        if(bit == 0)
            b[i] = '0';
        else
            b[i] = '1';
    }
    return b;
}

char* convert2_16bit(int a)
{
    int i = 0;
    int bit;
    char* c = (char*)malloc(16+1);
    for(i = 0; i < 16; i++)
    {
        bit = a & (1<<(15-i));
        if(bit == 0)
            c[i] = '0';
        else
            c[i] = '1';
    }
    return c;
}

int register_convert(char *reg)
{
    return atoi(&reg[1]);
}

unsigned int imm_convert(const char* buf)
{
    const char* to_conv = buf;
    int base = 10;
    int positive = 1;
    if(strlen(to_conv) >= 1 && to_conv[0] == '-')
    {
        positive = 0;
        to_conv = &to_conv[1];
    }

    if(strlen(to_conv) >= 2 &&
            to_conv[0] == '0' &&
            to_conv[1] == 'x')
    {
        base = 16;
        to_conv = &to_conv[2];
    }

    unsigned int tval = strtoul(to_conv, NULL, base);
    if(!positive) tval = -tval;
    return tval;

}


void int2bits(char* buf, unsigned int target)
{
       int j;
       for(j = 0; j < 32; j++)
       {
           buf[31-j] = '0' + ( target & 1 ) ;
           target >>= 1;
       }
       buf[32] = '\x00';
}
