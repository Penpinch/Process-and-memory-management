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

    // --- temp for test
    int genereated_procceses = 0, // stop condition
        finished_processes = 0;
    // --- temp for test

    resetCurrentPid();
    memoryInit(&memory);
    schedulerInit(&scheduler, settings.scheduler_quantum);
    queueInit(&new_processes);

    while(finished_processes < 15){ // --- Stop condition shound be an usuer interruption ---
        if(genereated_procceses < 15){
            Process *process = generateProcess(&settings, settings.system_quantum);

            if(process != NULL){
                if(enqueue(&new_processes, process)){
                    printf("[tick %d] P%d generado\n", system_tick, process->pid);
                    genereated_procceses++;
                } else { processDestroy(&process); }
            }
        }

// --------------------------------------

        while(!isEmpty(&new_processes)){ // if different than stop condition
            Process *process = queueFront(&new_processes);

            if(!placeInMemory(&memory, process)){ break; }

            process = dequeue(&new_processes);
            schedulerAddProcess(&scheduler, process);
            printf("[tick %d] P%d admitido en READY\n", system_tick, process->pid);
        }

// --------------------------------------

        if(scheduler.current_process == NULL){
            Process *next_process = schedulerAttend(&scheduler);

            if(next_process != NULL){ // There're not process on the ready queue. Possible deadlock
                next_process->state = RUNNING;
                scheduler.current_process = next_process;
            }

            printf("[tick %d] Ejecutando P%d (rem=%d, q=%d)\n", system_tick, next_process->pid, next_process->remaining_time, scheduler.quantum_remaining);
        }

// --------------------------------------

        SchedulerResults sch_results = schedulerTick(&scheduler);
        Process *current = scheduler.current_process;

        if(sch_results != SCHEDULER_RUNNING){
            printf("[tick %d] P%d | estado_tick=%s | rem=%d | q_rem=%d\n", system_tick, current->pid, resultToString(sch_results), current->remaining_time, scheduler.quantum_remaining);
        }

// --------------------------------------

        switch(sch_results){
            case SCHEDULER_RUNNING: { break; }
            case SCHEDULER_NO_PROCESSES: { break; }
            case SCHEDULER_QUANTUM_EXPIRED: {
                                                printf("[tick %d] Quantum agotado para P%d, reencolando...\n", system_tick, current->pid); 
                                                schedulerPreempt(&scheduler);
                                                break;
                                            }
            case SCHEDULER_PROCESS_FINISHED: {
                                                Process *done = schedulerTerminateProcess(&scheduler);

                                                if(done != NULL){
                                                    printf("[tick %d] P%d terminado (wait=%d)\n", system_tick, done->pid, done->waiting_time);
                                                    finished_processes++;
                                                    memoryFree(&memory, done);
                                                    processDestroy(&done);
                                                }
                                                break;                                
                                             }
        }

// -------------------------------------------
        
        system_tick++;
    }

    printf("\nSimulación terminada en %d ticks. Context switches: %d\n", system_tick, scheduler.context_switchs);

    deleteQueue(&new_processes);
    schedulerDestroy(&scheduler);
    memoryDestroy(&memory);

    return 0;
}
