#include <stdlib.h>
#include <stdbool.h>
#include "../headers/queue.h"

void queueInit(ProcessQueue *queue){
    queue->front = NULL;
    queue->back = NULL;
    queue->size = 0;
}

bool isEmpty(const ProcessQueue *queue){ return queue->front== NULL; }

Process *queueFront(const ProcessQueue *queue){
    if(isEmpty(queue)){ return NULL; }
    return queue->front->process;
}

bool enqueue(ProcessQueue *queue, Process *process){
    Node *new_node = malloc(sizeof(Node));

    if(new_node == NULL){ return false; }

    new_node->process = process;
    new_node->next = NULL;

    if(isEmpty(queue)){
        queue->front = new_node;
        queue->back = new_node;
    } else {
        queue->back->next = new_node;
        queue->back = new_node;
    }
    queue->size++;
    return true;
}

Process *dequeue(ProcessQueue *queue){
    if(isEmpty(queue)){ return NULL; }
    
    Node *temporal = queue->front;
    Process *process = temporal->process;

    queue->front = temporal->next;
    
    if(queue->front == NULL){ queue->back = NULL; }
    free(temporal);
    queue->size--;
    return process;
}

void deleteQueue(ProcessQueue *queue){
    while(!isEmpty(queue)){ dequeue(queue); }
}
