/*
To run the tests: run test_stack
*/ 

#include <assert.h> // for the tests

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "allocate.h"
#include <unistd.h>

sp test_stack;

void* send_command(void * arg)
{
    char* toDo = *(char**)arg;
    static int thread_num = 1;
    printf("%s\n",toDo);
    
    if (strncmp("PUSH ", toDo, 5) == 0){
            char * tok;
            tok = strtok(toDo, "\n");
            tok += 5;
            push(test_stack, tok, 0);
    }
    printf("thread %i finished\n", thread_num);
    thread_num+=1;
}


int main()
{
    // create new stack
    test_stack = create_stack(2);

    // the stack now will sleep for 20 seconds when a thread inside any of it's functions
    test_stack->wait = 0;

    pthread_t tid; //first thread

    char* command1 = "PUSH 1";

    // create new thread
    if( pthread_create(&tid, NULL, send_command, &command1) != 0){
            perror("Thread creating failed");
        }

    printf("First thread is in the stack, pushed 1\n"); 

    int len = strlen(command1);
    char* top = (char*) malloc(sizeof(char)* (len +1));
    sleep(1);
    top = peek(test_stack); //1
    printf("top is %s\n",top);

    pthread_t tid2,tid3; 

    char* command2 = "PUSH 2"; 
    char* command3 = "PUSH 3"; 

    
    test_stack->wait = 10;    
    // create new thread
    if( pthread_create(&tid2, NULL, send_command, &command2) != 0){
            perror("Thread creating failed");
        }
    

    printf("Second thread enter the stack, pushing 2\n");
    

    // create new thread
    if( pthread_create(&tid3, NULL, send_command, &command3) != 0){
            perror("Thread creating failed");
        }

    printf("third thread tries enter the stack, pushing 3\n");
    //test_stack->wait = 0;

    top = peek(test_stack); //2
    printf("TOP is %s\n", top);   //it won't be 3 because 3 waiting till 2 finish
    assert(strncmp(top, "2", 2)==0);

    sleep(10);

    //now second thread can enter, the first finished
    printf("after 10 seconds\n");
    
    top = peek(test_stack);
    printf("TOP is %s\n", top);    
    assert(strncmp(top, "3", 3)==0);

    return 0;
}



