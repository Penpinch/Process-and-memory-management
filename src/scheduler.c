#include "../headers/process.h"
#include "../headers/scheduler.h"

#include <stddef.h>
#include <stdbool.h>

void schedulerInit(Scheduler *scheduler, int quantum){
    queueInit(&scheduler->ready_queue);
    scheduler->quantum = quantum;
    scheduler->quantum_remaining = 0;
    scheduler->context_switchs = 0;
    scheduler->last_pid = -1;
    scheduler->current_process = NULL;
}

void schedulerAddProcess(Scheduler *scheduler, Process *process){
    bool success = enqueue(&scheduler->ready_queue, process);

    if(success){ process->state = READY; }
}

Process *schedulerAttend(Scheduler *scheduler){
    Process *process = dequeue(&scheduler->ready_queue);

    if(process == NULL){ return NULL; }
    if(scheduler->last_pid != -1 && scheduler->last_pid != process->pid){ scheduler->context_switchs++; }

    scheduler->last_pid = process->pid;
    scheduler->quantum_remaining = scheduler->quantum;
    return process;
}

SchedulerResults schedulerTick(Scheduler *scheduler){
    if(scheduler->current_process == NULL){ return SCHEDULER_NO_PROCESSES; }

    Process *current = scheduler->current_process;
    current->remaining_time--;
    scheduler->quantum_remaining--;

    if(current->remaining_time == 0){ return SCHEDULER_PROCESS_FINISHED; }
    if(scheduler->quantum_remaining == 0){ return SCHEDULER_QUANTUM_EXPIRED; }
    return SCHEDULER_RUNNING;
}

void schedulerPreempt(Scheduler *scheduler){ // Take the process out of the cpu.
     if(scheduler->current_process == NULL){ return; }
    
    Process *process = scheduler->current_process;                                                                          
    process->state = READY;
    
    enqueue(&scheduler->ready_queue, process);
    scheduler->current_process = NULL;
}

Process *schedulerTerminateProcess(Scheduler *scheduler){
    if(scheduler->current_process == NULL){ return NULL; }

    Process *finished_process = scheduler->current_process;
    finished_process->state = TERMINATED;
    scheduler->current_process = NULL;

    return finished_process;
}

void schedulerDestroy(Scheduler *scheduler){
    deleteQueue(&scheduler->ready_queue);
    scheduler->current_process = NULL;
    scheduler->quantum = 0;
    scheduler->quantum_remaining = 0;
    scheduler->context_switchs = 0;
}
