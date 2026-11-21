#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "../headers/process.h"
#include "../headers/queue.h"

typedef struct {
    ProcessQueue ready_queue;
    Process *current_process;
    int quantum, 
        quantum_remaining, 
        context_switchs;
} Scheduler;

typedef enum {
    SCHEDULER_NO_PROCESSES,
    SCHEDULER_RUNNING,
    SCHEDULER_QUANTUM_EXPIRED,
    SCHEDULER_PROCESS_FINISHED
} SchedulerResults;

void schedulerInit(Scheduler *scheduler, int quantum);

void schedulerAddProcess(Scheduler *scheduler, Process *process);

Process *schedulerAttend(Scheduler *scheduler);

void schedulerUpdateWaitTime(Scheduler *scheduler);

SchedulerResults schedulerTick(Scheduler *scheduler);

void schedulerPreempt(Scheduler *scheduler);

Process *schedulerTerminateProcess(Scheduler *scheduler);

void schedulerDestroy(Scheduler *scheduler);

#endif
