#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define SKIP_HEADER(p) ((void *)((size_t)p + sizeof(block))) 
#define GET_HEADER(p) ((void *)((size_t)p - sizeof(block))) 
#define BLOCK_SIZE sizeof(block)

typedef struct block{

    size_t size;
    struct block * next;
    
}block, *pblock; 

pblock head = NULL; 

void remove_block(pblock p){

    
    if (!head){
        printf("Trying to delete from empty list\n");
        return;
    }
    pblock curr = head->next, prev = head;
    //in case p is the head
    if(p == head){
        head = head->next;
        return;
    }
    //traverse the list untill we find it
    while(curr){

        if(curr == p){
            prev->next = curr->next;
        }
        prev = curr;
        curr = curr->next;

    }

}

void * mymalloc(size_t size){

    pblock curr = head;
    

    // we look for an empty block big enought for our purpose
    while(curr){
                
        if(curr->size == size){// the block fits perfectly
            remove_block(curr);
            return SKIP_HEADER(curr);
        }

        // the block is bigger then what we need, we will split it to 2 blocks
        
        if(curr->size > size + BLOCK_SIZE){
            
            pblock newblock = (pblock) ((size_t)SKIP_HEADER(curr) + size); //create the new block and add it after curr
            newblock->next = curr->next;
            curr->next = newblock;
            remove_block(curr);

            //setting the new sizes for the blocks
            newblock->size = curr->size - size - BLOCK_SIZE;
            curr->size = size;
            
            return SKIP_HEADER(curr);
        }
        curr = curr->next;

    }

        // if we got here there are no fitting free blocks
        curr = (pblock)sbrk(size + sizeof(block));
        if(!curr){
            return NULL;
        }
        
        curr->next = NULL;
        curr->size = size;

        return (void *) ((size_t)curr + sizeof(block)); // skips the header

}

void myfree(void * p){

    pblock free_block = (pblock) GET_HEADER(p);
    // add the free blcok to the head of the list
    free_block->next = head; 
    head = free_block;

}

void * myrealloc(void * tomove, size_t new_size){

    // getting a new block with the required size
    pblock p =(pblock) mymalloc(new_size);
    p->size = new_size;
    p->next = NULL;
    
    // moving the old content the new block
    void * header = GET_HEADER(tomove);
    memcpy(SKIP_HEADER(p), tomove, ((pblock)header)->size);
    myfree(tomove);

    return SKIP_HEADER(p);
}