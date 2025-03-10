#include <sys/_types/_pid_t.h>
#include <stdlib.h>
#include "cirque.h"

// enqueue, dequeue, 'move first element to back'
void enqueue(queue *q, pid_t id) {
    node *n = malloc(sizeof(node));
    n->pid = id;
    n->next = NULL;

    if (q->end == NULL) {
        q->front = n;
        q->end = n;
    } else {
        q->end->next = n;
        q->end = n;
    } 
}

pid_t dequeue(queue *q) {
    if (q->front != NULL) {
        node *temp = q->front;
        pid_t pid = temp->pid;
        q->front = q->front->next;

        if (q->front == NULL) {
            q->end == NULL;
        }

        free(temp);
        return pid;
    }
}