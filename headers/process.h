#ifndef PROCESS_H
#define PROCESS_H

typedef enum { NEW, READY, RUNNING, BLOCKED, TERMINATED } ProcessState;

typedef struct {
    int pid;
    int memory_required;
    int execution_tmie;
    int remaining_time;
    int arrival_time;
    int waiting_time;
    ProcessState state;
} Process;

#endif
