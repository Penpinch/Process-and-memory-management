#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include "../headers/process.h"

typedef struct Node {
    Process *process;
    struct Node *next;
} Node;

typedef struct {
    Node *front, *back;
    int size;
} ProcessQueue;

void queueInit(ProcessQueue *queue);

bool isEmpty(const ProcessQueue *queue);

Process *queueFront(const ProcessQueue *queue);

bool enqueue(ProcessQueue *queue, Process *process);

Process *dequeue(ProcessQueue *queue);

void deleteQueue(ProcessQueue *queue);

#endif
