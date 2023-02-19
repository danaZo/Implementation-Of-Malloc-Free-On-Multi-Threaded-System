#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "allocate.h"
#include <unistd.h>

#define FALSE 0
#define TRUE 1


sp create_stack(size_t size){

    sp st = (sp)mymalloc(sizeof(stack));
    st->s = (char **) mymalloc(sizeof(char*) * size);
    if(!st || !st->s){
        perror("Stack creation");
        exit(1);
    }
    st->size = size;
    st->TOP = -1;
    st->wait = 0;

    // create a unique lock for the stack
    if(pthread_mutex_init(&st->slock,NULL)){
        perror("Lock creation failed");
        exit(1);
    }

    return st;
}
// add new element to the top of the stack
void push(sp st, char * str, int gotLock){
    if(!gotLock){
        pthread_mutex_lock(&st->slock);
    }

    sleep(st->wait);

    //check if stack is full , and resized if needed
    if(st->size == st->TOP +1){
        printf("Stack full, resizing...\n");
        st->s = (char **) myrealloc(st->s,sizeof(char*) * (st->size += 10));
        if(!st->s){
            perror("Resizing stack");
            exit(1);
        }
    }
    st->TOP++;
    int len = strlen(str);

    st->s[st->TOP] = (char*) mymalloc(sizeof(char) * (1 + len));
    strcpy(st->s[st->TOP],str);

      if(!gotLock){
        pthread_mutex_unlock(&st->slock);
    }
}
// add new element to the bottom of the stack

void enque(sp st, char * str){

    pthread_mutex_lock(&st->slock);
    //create temp stack
    sleep(st->wait);
    sp temp = create_stack(st->size);
    //move the elements to the new stack
    char * tstr;
    while(!isEmpty(st)){
        tstr = pop(st, TRUE);
        push(temp,tstr , 1);
    }
    //add the new element
    push(st, str, 1);
    //return the old element back
    while(!isEmpty(temp)){
        tstr = pop(temp, TRUE);
        push(st,tstr, 1);
    }
    pthread_mutex_unlock(&st->slock);
}

// remove the top of the stack
char * pop(sp st, int hasLock){

    if(!hasLock){
        pthread_mutex_lock(&st->slock);
    }
    sleep(st->wait);

    if(isEmpty(st)){
        return NULL;
    }

    int len = strlen(st->s[st->TOP]);
    char *topStr = (char*) mymalloc(sizeof(char)* len +1);
    if(!topStr){
        perror("str allocate[pop]");
        exit(1);
    }
    strcpy(topStr, st->s[st->TOP]);
    myfree(st->s[st->TOP]);
    --st->TOP;

    if(!hasLock){
        pthread_mutex_unlock(&st->slock);
    }

    return topStr;
}

char * deque(sp st){
    char * res = NULL;
    pthread_mutex_lock(&st->slock);
    sleep(st->wait);
    //create temp stack
    sp temp = create_stack(st->size);
    //move the elements to the new stack
    char * tstr;
    while(!isEmpty(st)){
        tstr = pop(st, TRUE);
        push(temp,tstr , 1);
    }
    //drop the last element
    res = pop(temp, 1);
    //return the old element back
    while(!isEmpty(temp)){
        tstr = pop(temp, TRUE);
        push(st,tstr, 1);
    }
    pthread_mutex_unlock(&st->slock);
    return res;
}

// return the top of the stack without removing it
char *peek(sp st){

    pthread_mutex_lock(&st->slock);

    if(isEmpty(st)){
        pthread_mutex_unlock(&st->slock);
        return NULL;
    }
    sleep(st->wait);
    int len = strlen(st->s[st->TOP]);
    char *topStr = (char*) malloc(sizeof(char)* len +1);
    if(!topStr){
        perror("str allocate[pop]");
        exit(1);
    }
    strcpy(topStr, st->s[st->TOP]);



    pthread_mutex_unlock(&st->slock);

    return topStr;
}

int isEmpty(sp st){

    return st->TOP == -1;
}

void print_stack(sp st){

    if(isEmpty(st)){
        printf("Stack is empty\n");
    }

    for(int i = 0 ; i <= st->TOP; i++){

        printf("%s ~ ", st->s[i]);
        fflush(stdout);
    }
    printf("\n");
}


