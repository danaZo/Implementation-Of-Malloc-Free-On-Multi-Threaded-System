#pragma once


typedef struct block{

    size_t size;
    struct block * next, *prev;
    
}block, *pblock; 

void * mymalloc(unsigned long size);

void * myrealloc(void * tomove, size_t new_size);

void myfree(void *);

void remove_block(pblock);