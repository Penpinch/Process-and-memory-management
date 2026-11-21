#include "../headers/settings.h"
#include "../headers/process.h"
#include "../headers/scheduler.h"
#include "../headers/memory.h"
#include "../headers/terminal.h"

#include <stdio.h>
#include <stdlib.h>

int main(void){
    Scheduler scheduler;
    Settings settings = defaultSettings();
    Memory memory;
    ProcessQueue new_processes;
    int system_tick = 0;

    resetCurrentPid();
    memoryInit(&memory);
    schedulerInit(&scheduler, settings.system_quantum);
    queueInit(&new_processes);

    int num_process = 0; // Temporal as a stop condition

    while(num_process < 15){ // --- Stop condition shound be an usuer interruption ---

        if(isEmpty(&new_processes)){ // if different than stop condition
            Process *process = generateProcess(&settings, system_tick);
            enqueue(&new_processes, process);
            placeInMemory(&memory, process);
            schedulerAddProcess(&scheduler, dequeue(&new_processes));
        }

        if(scheduler.current_process == NULL){
            Process *next_process = schedulerAttend(&scheduler);

            if(next_process == NULL){ break; } // There're not process on the ready queue
            
            next_process->state = RUNNING;
            scheduler.current_process = next_process;
            scheduler.context_switchs++;

            printf("[tick %d] Despachando P%d (rem=%d, q=%d)\n", system_tick, next_process->pid, next_process->remaining_time, scheduler.quantum_remaining);
        }

        SchedulerResults sch_results = schedulerTick(&scheduler);
        system_tick++;

        Process *current = scheduler.current_process;
        printf("  tick %d -> P%d | estado_tick=%s | rem=%d | q_rem=%d\n", system_tick, current ? current->pid : -1, resultToString(sch_results), 
                current ? current->remaining_time : 0, scheduler.quantum_remaining);

        if(sch_results == SCHEDULER_PROCESS_FINISHED){
            Process *done = schedulerTerminateProcess(&scheduler);
            memoryFree(&memory, done);

            printf("  [tick %d] P%d terminado (wait=%d)\n", system_tick, done->pid, done->waiting_time);
        } else if(sch_results == SCHEDULER_QUANTUM_EXPIRED){
            printf("  [tick %d] Quantum agotado para P%d, reencolando...\n", system_tick, current->pid); 
            schedulerPreempt(&scheduler);
            scheduler.context_switchs++;
        }

        num_process++;
    }

    printf("\nSimulación terminada en %d ticks. Context switches: %d\n", system_tick, scheduler.context_switchs);
    schedulerDestroy(&scheduler);

    return 0;
}
