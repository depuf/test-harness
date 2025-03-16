#include <stdlib.h>
#include <stdio.h>
#include "p1fxns.h"
#include "cirque.h"

queue *create_queue() {
    queue *q = malloc(sizeof(queue));
    if (q == NULL) {
        p1perror(1, "error: failed to allocate memory for queue\n");
        return NULL;
    }

    q->front = NULL;
    q->end = NULL;
    return q;
}

void destroy_queue(queue *q) {
    if (q == NULL) {
        return;
    }

    node *curr = q->front;
    while (curr != NULL) {
        node *temp = curr;
        curr = curr->next;
        free(temp);
    }
    free(q);
}

void enqueue(queue *q, pid_t id) {
    if (q == NULL) {
        p1perror(1, "error: queue is null\n");
        return;
    }

    node *n = malloc(sizeof(node));
    if (n == NULL) {
        p1perror(1, "error: failed to allocate node\n");
        return;
    }

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
    if (q == NULL || q->front == NULL) {
        return -1; 
    }

    node *temp = q->front;
    pid_t pid = temp->pid;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->end = NULL;  
    }

    free(temp);
    return pid;
}

void remove_from_queue(queue *q, pid_t pid) {
    if (q == NULL || q->front == NULL) {
        return;
    }

    node *prev = NULL;
    node *curr = q->front;

    while (curr != NULL) {
        if (curr->pid == pid) {
            if (prev == NULL) {
                q->front = curr->next;
                if (q->front == NULL) {
                    q->end = NULL; 
                }
            } else {
                prev->next = curr->next;
                if (curr->next == NULL) {
                    q->end = prev;  
                }
            }

            free(curr);
            return;
        }

        prev = curr;
        curr = curr->next;
    }

}

int is_empty(queue *q) {
    if (q == NULL) {
        return 1; 
    }
    return q->front == NULL; 
}