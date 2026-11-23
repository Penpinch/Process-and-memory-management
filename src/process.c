#include "../headers/process.h"
#include "../headers/settings.h"
#include <stdlib.h>
#include <stdbool.h>

static int current_pid = 1;

Process* generateProcess(const Settings *s, int current_time){
    Process* process = malloc(sizeof(Process));
    if (process == NULL) return NULL;

    // PID
    process->pid = current_pid;
    current_pid++;
    // MEMORY
    process->memory_required = MIN_SPLIT + rand() % (s->max_memory_process - MIN_SPLIT + 1);
    // EXECUTION TIME
    process->execution_time = 1 + rand() % s->scheduler_quantum;
    // REMAINING TIME
    process->remaining_time = process->execution_time;
    // ARRIVAL TIME
    process->arrival_time = current_time;
    // WAITING TIME
    process->waiting_time = 0;
    // FINISH TIME
    process->finish_time = 0;
    // STATE
    process->state = NEW;

    return process;
}
void processDestroy(Process** process){
    if (process == NULL || *process == NULL) return;
    free(*process);
    *process = NULL;
}

void resetCurrentPid() { current_pid = 1; }

bool processFinished(const Process *p) { return (p->remaining_time <= 0); }
int processRun(Process *p, int system_quantum){
    p->state = RUNNING;
    if (p->remaining_time - system_quantum < 0){
        int run_time = p->remaining_time;
        p->remaining_time = 0;
        return run_time;
    } // if

    p->remaining_time -= system_quantum;
    return system_quantum;
}
void processSetFinish(Process *p, int time) { 
    p->finish_time = time;
    p->waiting_time = p->finish_time - p->arrival_time - p->execution_time; 
    p->state = TERMINATED;
}
