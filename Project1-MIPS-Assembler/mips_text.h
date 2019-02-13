#ifndef __MIPS_TEXT_H__
#define __MIPS_TEXT_H__

#include <stdio.h>
#include "mips_parser.h"
#include "mips_generator.h"

void type_R(struct mips_generator* generator, int t_argc, char** t_argv);

void type_I(struct mips_generator* generator, int t_argc, char** t_argv);

void type_J(struct mips_generator* generator, int t_argc, char** t_argv); 

void type_pseudo(struct mips_generator* generator, int t_argc, char** t_argv);

#endif 
