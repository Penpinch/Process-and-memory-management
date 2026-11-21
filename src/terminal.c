/*
- Draw memory.
- Show process queues.
- User input.
*/

#include <stdio.h>
#include "../headers/terminal.h"
#include "../headers/scheduler.h"

const char *resultToString(SchedulerResults sch_results){
    switch(sch_results){
        case SCHEDULER_NO_PROCESSES: return "No processes";
        case SCHEDULER_RUNNING: return "Running";
        case SCHEDULER_QUANTUM_EXPIRED: return "Quantum expired";
        case SCHEDULER_PROCESS_FINISHED: return "Process finished";
        default: return "Unknowd";
    }
}

void printMemory(const Memory *m){
    printf(" Memory(%d blocks, %d total): ", m->block_count, TOTAL_MEMORY);
    for(int i = 0; i < m->block_count; i++){
        if(m->blocks[i].process == NULL){ printf("[0,%d,0]", m->blocks[i].size); }
        else {
            printf("[%d,%d,%d]", m->blocks[i].process->pid, m->blocks[i].size, m->blocks[i].process->remaining_time);
        }
    }
    printf("\n");
}
