#ifndef PROCESS_H
#define PROCESS_H

#include "settings.h"
#include <stdbool.h>

typedef enum { NEW, READY, RUNNING, TERMINATED } ProcessState;

typedef struct {
    int pid;
    int memory_required;
    int execution_time;
    int remaining_time;
    int arrival_time;
    int waiting_time;
    int finish_time; 
    ProcessState state;
} Process;

Process* generateProcess(const Settings*, int);
void processDestroy(Process**);
void resetCurrentPid();

bool processFinished(const Process*);
int processRun(Process*, int);
void processSetFinish(Process*, int);

#endif