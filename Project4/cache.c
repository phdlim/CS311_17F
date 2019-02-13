#include "cache.h"
#include "util.h"

/* cache.c : Implement your functions declared in cache.h */


/***************************************************************/
/*                                                             */
/* Procedure: setupCache                  		       */
/*                                                             */
/* Purpose: Allocates memory for your cache                    */
/*                                                             */
/***************************************************************/

void setupCache(int capacity, int num_way, int block_size)
{
/*	code for initializing and setting up your cache	*/
/*	You may add additional code if you need to	*/
 	
	int i,j; //counter
	int nset=0; // number of sets
	int _wpb=0; //words per block   
	nset=capacity/(block_size*num_way);
	_wpb = block_size/BYTES_PER_WORD;

	Cache = (uint32_t  ***)malloc(nset*sizeof(uint32_t **));
  Tag = (uint32_t **)malloc(nset*sizeof(uint32_t *));
  Order = (uint32_t *)malloc(nset*sizeof(uint32_t));
	for (i=0;i<nset;i++) {
		Cache[i] = (uint32_t ** )malloc(num_way*sizeof(uint32_t*));
	  Tag[i] = (uint32_t *)malloc(num_way*sizeof(uint32_t));
  }
	for (i=0; i<nset; i++){	
		for (j=0; j<num_way; j++){
			Cache[i][j]=(uint32_t*)malloc(sizeof(uint32_t)*(_wpb));
		}
	}
  Order[0] = 0x00011011;
  Order[1] = 0x00011011;
  miss_penalty = 0;
}


/***************************************************************/
/*                                                             */
/* Procedure: setCacheMissPenalty                  	           */
/*                                                             */
/* Purpose: Sets how many cycles your pipline will stall       */
/*                                                             */
/***************************************************************/

void setCacheMissPenalty(int penalty_cycles)
{
/*	code for setting up miss penaly			*/
/*	You may add additional code if you need to	*/	
	  miss_penalty = penalty_cycles;
    
}

bool find_miss(uint32_t set, uint32_t address)
{
    int i;
    for(i = 0;i < 4;i++)
    {
        if((Tag[set][i] & 0xFFFFFFF) == (address >> 4))
            return 0;
    }
    return 1;
}

void Change_order(int way, int set)
{
    int order_way;
    switch(way)
    {
        case 0:
            order_way = 0x00;
            break;
        case 1:
            order_way = 0x01;
            break;
        case 2:
            order_way = 0x10;
            break;
        case 3:
            order_way = 0x11;
            break;
    }
    if(order_way == ((Order[set] & 0xFF000000) >> 24))
        Order[set] = ((Order[set] << 8) & 0xFFFFFFFF) | order_way;
    else if(order_way == ((Order[set] & 0x00FF0000) >> 16))
        Order[set] = (Order[set] & 0xFF000000) | ((Order[set] & 0xFFFF) << 8) | order_way;
    else if(order_way == ((Order[set] & 0x0000FF00) >> 8))
        Order[set] = (Order[set] & 0xFFFF0000) | ((Order[set] & 0xFF) << 8) | order_way;
}

void Store_Cache(struct_data Memory_address)
{
    int tag;
    int set;
    int way;
    int order_way;
    int byte;
    
    tag = Memory_address.address >> 4;
    set = (Memory_address.address >> 3) & 0x1;
    byte = (Memory_address.address >> 2) & 0x1;
    order_way = ((Order[set] & 0xFF000000) >> 24);
    switch(order_way)
    {
        case 0x00:
            way = 0;
            break;
        case 0x01:
            way = 1;
            break;
        case 0x10:
            way = 2;
            break;
        case 0x11:
            way = 3;
            break;
    }
    if(Tag[set][way] >> 31)
        mem_write_block(((Tag[set][way] << 4) | (set << 3)), Cache[set][way]);
    if(!Memory_address.check_SW)
    {
        mem_read_block(Memory_address.address, Cache[set][way]);
        Tag[set][way] = tag;
        Order[set] = ((Order[set] << 8) & 0xFFFFFFFF) | order_way;
    }
    else
    {
        mem_read_block(Memory_address.address, Cache[set][way]);
        //Cache[set][way][byte] = Memory_address.data;
        Tag[set][way] = tag | 0x80000000;
        Order[set] = ((Order[set] << 8) & 0xFFFFFFFF) | order_way;
    }
}

cache_data Use_Cache_LW(uint32_t address)
{
    struct cache_data data;
    struct struct_data Memory_address;
    int set;
    int miss;
    int way;
    int i;
    set = (address >> 3) & 1;
    miss = find_miss(set, address);
    Memory_address.address = address;
    Memory_address.check_SW = 0;
    if(miss && (miss_penalty < 29))
    {
        data.check = 0;
        miss_penalty++;
    }
    else if(miss && (miss_penalty == 29))
    {
        Store_Cache(Memory_address);
        data.check = 0;
        miss_penalty++;
    }
    else
    {
        for(i = 0;i < 4;i++)
            if((Tag[set][i] & 0xFFFFFFF) == (address >> 4))
            { 
                way = i;
                break;
            }
        data.data[0] = Cache[set][way][0];
        data.data[1] = Cache[set][way][1];
        data.check = 1;
        miss_penalty = 0;
        Change_order(way, set);
    }
    return data;
}

bool Use_Cache_SW(int address, int data)
{
    struct struct_data Memory_address;
    int miss;
    int set;
    int way;
    int byte;
    int i;

    set = (address >> 3) & 1;
    miss = find_miss(set, address);
    Memory_address.address = address;
    Memory_address.data = data;
    byte = (address >> 2) & 0x1;
    Memory_address.check_SW = 1;
    if(miss && (miss_penalty < 29))
    {
        miss_penalty++;
        return 0;
    }
    else if(miss && (miss_penalty == 29))
    {
        Store_Cache(Memory_address);
        miss_penalty++;
        return 0;
    }
    else
    {
        for(i = 0;i < 4;i++)
            if((Tag[set][i] & 0x7FFFFFFF) == (address >> 4))
            { 
                way = i;
                break;
            }
        Cache[set][way][byte] = data;
        miss_penalty = 0;
        Change_order(way, set);
        return 1;
    }
}

/* Please declare and implement additional functions for your cache */
