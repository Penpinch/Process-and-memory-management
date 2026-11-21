/*
- Draw memory.
- Show process queues.
- User input.
*/

#include "../headers/terminal.h"
#include "../headers/scheduler.h"

const char *resultToString(SchedulerResults sch_results){
    switch(sch_results){
        case SCHEDULER_NO_PROCESSES: return "NO_PROCESSES";
        case SCHEDULER_RUNNING: return "RUNNING";
        case SCHEDULER_QUANTUM_EXPIRED: return "QUANTUM_EXPIRED";
        case SCHEDULER_PROCESS_FINISHED: return "PROCESS_FINISHED";
        default: return "UNKNOWN";
    }
}
