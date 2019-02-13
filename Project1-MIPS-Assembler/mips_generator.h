#ifndef __MIPS_GENERATOR_H__
#define __MIPS_GENERATOR_H__

struct label_list
{
    unsigned int addr;
    char* name;
    struct label_list* next;
};

struct promise_list
{
    unsigned int cnt;
    char* name;
    int start_bit;
    int end_bit;
    struct promise_list* next;
};

struct mips_generator
{
    unsigned int data_base;
    unsigned int data_cnt;
    unsigned int data_cap;
    unsigned int text_base;
    unsigned int text_cnt;
    unsigned int text_cap;
    unsigned int* data_buf;
    char** text_buf;
    struct label_list* l_list;
    struct promise_list* p_list;
};

struct mips_generator* new_mips_generator();

void del_mips_generator();

void mips_generator_insert_data(struct mips_generator* this, int data);
void mips_generator_insert_text(struct mips_generator* this, char* text);
void mips_generator_promise_text(struct mips_generator* this, char* text, char* name, int start, int end);

void mips_generator_register_data_label(struct mips_generator* this, char* name);
void mips_generator_register_text_label(struct mips_generator* this, char* name);

unsigned int mips_generator_get_label(struct mips_generator* this, char* name);

void mips_generator_commit_all(struct mips_generator* this);
void mips_generator_emit_code(struct mips_generator* this, char* fname);

#endif
