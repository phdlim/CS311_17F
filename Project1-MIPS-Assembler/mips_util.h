#ifndef __MIPS_UTIL_H__
#define __MIPS_UTIL_H__

char* convert2_5bit(int a);
char* convert2_16bit(int a);
int register_convert(char* reg);
unsigned int imm_convert(const char* buf);
void int2bits(char* buf, unsigned int target);

#endif
