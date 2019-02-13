#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "mips_generator.h"

struct mips_generator* new_mips_generator()
{
    struct mips_generator* this = (struct mips_generator*)malloc(sizeof(struct mips_generator));
    this->data_base = 0x10000000;
    this->data_cnt = 0;
    this->data_cap = 8;
    this->text_base = 0x400000;
    this->text_cnt = 0;
    this->text_cap = 8;
    this->data_buf = (unsigned int*)malloc(sizeof(unsigned int) * this->data_cap);
    this->text_buf = (char**)malloc(sizeof(char*) * this->text_cap);
    this->l_list = NULL;
    this->p_list = NULL;
}

static void mips_generator_add_label(struct mips_generator* this, unsigned int addr, char* name)
{
    //printf("ADD LABEL : %s -> %p\n", name, (void*)addr);
    struct label_list* to_add = (struct label_list*)malloc(sizeof(struct label_list));
    to_add->addr = addr;
    to_add->name = strdup(name);
    to_add->next = NULL;
    if(!this->l_list)
    {
        this->l_list = to_add;
        return;
    }
    struct label_list* iter = this->l_list;
    for(; iter->next; iter=iter->next);
    iter->next = to_add;
    return;
}

static void mips_generator_add_promise(struct mips_generator* this, char* name, int start_bit, int end_bit)
{
    struct promise_list* to_add = (struct promise_list*)malloc(sizeof(struct promise_list));
    to_add->cnt = this->text_cnt;
    to_add->name = strdup(name);
    to_add->start_bit = start_bit;
    to_add->end_bit = end_bit;
    to_add->next = NULL;

    if(!this->p_list)
    {
        this->p_list = to_add;
        return;
    }
    struct promise_list* iter = this->p_list;
    for(; iter->next; iter=iter->next);
    iter->next = to_add;
    return;


}

void del_mips_generator()
{
}

void mips_generator_insert_data(struct mips_generator* this, int data)
{
    if(this->data_cnt >= this->data_cap)
    {
        this->data_cap *= 2;
        this->data_buf = (unsigned int*)realloc( this->data_buf, sizeof(unsigned int) * this->data_cap);
    }
    this->data_buf[this->data_cnt] = data;
    //printf("INSERT DATA : %p -> %x\n", (void*)(this->data_base + this->data_cnt * 4), data);
    this->data_cnt++;
}

void mips_generator_insert_text(struct mips_generator* this, char* text)
{
    assert(strlen(text) == 32);
    if(this->text_cnt >= this->text_cap)
    {
        this->text_cap *= 2;
        this->text_buf = (char**)realloc( this->text_buf, sizeof(char*) * this->text_cap);
    }
    this->text_buf[this->text_cnt] = text;
    this->text_cnt++;
}

void mips_generator_promise_text(struct mips_generator* this, char* text, char* name, int start, int end)
{
    assert(strlen(text) == 32);
    if(this->text_cnt >= this->text_cap)
    {
        this->text_cap *= 2;
        this->text_buf = (char**)realloc( this->text_buf, sizeof(char*) * this->text_cap);
    }
    mips_generator_add_promise(this, name, start, end);
    this->text_buf[this->text_cnt] = text;
    this->text_cnt++;
}

void mips_generator_register_data_label(struct mips_generator* this, char* name)
{
    mips_generator_add_label(this, this->data_base + this->data_cnt * 4, name);
}

void mips_generator_register_text_label(struct mips_generator* this, char* name)
{
    mips_generator_add_label(this, this->text_base + this->text_cnt * 4, name);
}

unsigned int mips_generator_get_label(struct mips_generator* this, char* name)
{
    struct label_list *iter;
    for(iter = this->l_list; iter; iter = iter->next)
    {
        if(!strcmp(iter->name, name))
        {
            return iter->addr;
        }
    }
    return 0;
}

void mips_generator_commit_all(struct mips_generator* this)
{
    struct promise_list* iter;
    for(iter = this->p_list; iter; iter = iter->next)
    {
        //printf("SEARCH %s\n", iter->name);
        unsigned int l_addr = mips_generator_get_label(this, iter->name);
        assert(l_addr);
        //printf("%p\n", l_addr);
        char* target = this->text_buf[iter->cnt];
        //printf("%s\n", target);
        //printf("%d\n",iter->end_bit - iter->start_bit + 1);
        if(iter->start_bit == 16)
        {
            unsigned int put_addr = (-(this->text_base + iter->cnt * 4 + 4) + l_addr) >> 2;
            int i;
            for(i = 0; i < 16; i++)
            {
                target[iter->start_bit + 15 - i] = '0' + (put_addr & 1);
                put_addr >>= 1;
            }
        }
        else if(iter->start_bit == 6)
        {
            unsigned int put_addr = l_addr >> 2;
            int i;
            for(i = 0; i < 26; i++)
            {
                target[iter->start_bit + 25 - i] = '0' + (put_addr & 1);
                put_addr >>= 1;
            }
        }
        //printf("%s\n", target);
    }
}


void mips_generator_emit_code(struct mips_generator* this, char* fout_name)
{
    FILE* fout = fopen(fout_name, "w");
    int i , j;
    char buf[33];
    int2bits(buf, this->text_cnt * 4);
    fprintf(fout, "%s", buf);
    int2bits(buf, this->data_cnt * 4);
    fprintf(fout, "%s", buf);
    for(i = 0; i < this->text_cnt; i++)
    {
        fprintf(fout, "%s", this->text_buf[i]);
    }
    for(i = 0; i < this->data_cnt; i++)
    {
        int2bits(buf, this->data_buf[i]);
        fprintf(fout, "%s", buf);
    }
    fclose(fout);
}

