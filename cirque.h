#ifndef QUEUE_H
#define QUEUE_H

#include <sys/_types/_pid_t.h>
#include <stdlib.h>

typedef struct node {
    pid_t pid;             
    struct node *next;   
} node;

typedef struct queue {
    node *front;  
    node *end;    
} queue;

void enqueue(queue *q, pid_t id);
pid_t dequeue(queue *q);
void remove_from_queue(queue *q, pid_t pid);

#endif 