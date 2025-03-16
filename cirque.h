#ifndef QUEUE_H
#define QUEUE_H

#include <sys/types.h>
#include "p1fxns.h"

typedef struct node {
    pid_t pid;             
    struct node *next;   
} node;

typedef struct queue {
    node *front;  
    node *end;    
} queue;

queue *create_queue();
void destroy_queue(queue *q);
void enqueue(queue *q, pid_t id);
pid_t dequeue(queue *q);
void remove_from_queue(queue *q, pid_t pid);
int is_empty(queue *q);

#endif 