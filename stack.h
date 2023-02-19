#include <pthread.h>

typedef struct stack{

    int TOP, size, wait;
    char ** s;
    pthread_mutex_t slock;
    
}stack,*sp;

#define FALSE 0
#define TRUE 1

sp create_stack(size_t size);
void delete_stack(sp st);

char * pop(sp st, int hasLock);
void push(sp st, char * str, int gotLock);
void enque(sp st, char * str);
char *peek(sp st);
char * deque(sp st);

int isEmpty(sp st);
void print_stack(sp st);


