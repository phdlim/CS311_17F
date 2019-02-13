#ifndef _CACHE_H_
#define _CACHE_H_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* cache.h : Declare functions and data necessary for your project*/

int miss_penalty; // number of cycles to stall when a cache miss occurs
uint32_t ***Cache; // data cache storing data [set][way][byte]
uint32_t **Tag; // tag storing [set][way]
uint32_t *Order;

typedef struct struct_data
{
    uint32_t address;
    uint32_t data;
    uint32_t check_SW;
} struct_data;

typedef struct cache_data
{
    uint32_t data[2];
    bool check;
} cache_data;

void setupCache(int, int, int);
struct cache_data Use_Cache_LW(uint32_t address);
void setCacheMissPenalty(int);

#endif
