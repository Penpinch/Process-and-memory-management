#include "../headers/settings.h"
#include "../headers/process.h"
#include "../headers/scheduler.h"
#include "../headers/memory.h"
#include "../headers/terminal.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
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
        printf("\n-------------------------------- TICK %d --------------------------------\n", system_tick);
        if(genereated_procceses < 15){
            Process *process = generateProcess(&settings, settings.system_quantum);

            if(process != NULL){
                if(enqueue(&new_processes, process)){
                    printf("[NEW] P%d generado(mem=%d kb, rem=%d)\n", process->pid, process->memory_required, process->remaining_time);
                    genereated_procceses++;
                } else { processDestroy(&process); }
            }
        }
        printf("------------------------------------------------\n");

// --------------------------------------

        bool allocated_any = false;
        while(!isEmpty(&new_processes)){
            Process *process = queueFront(&new_processes);

            if(!placeInMemory(&memory, process)){
                printf(" [MEM] Sin espapcio suficiente para P%d(%d KB), se queda en cola NEW\n", process->pid, process->memory_required);
                break;
            }

            process = dequeue(&new_processes);
            schedulerAddProcess(&scheduler, process);

            allocated_any = true;
            printf(" [READY] P%d aceptado en READY(%d kb)\n", process->pid, process->memory_required);
        }

        if(allocated_any){ printMemory(&memory); }
        printf("------------------------------------------------\n");

// --------------------------------------

        if(scheduler.current_process == NULL){
            Process *next_process = schedulerAttend(&scheduler);

            if(next_process != NULL){ // There're not process on the ready queue. Possible lock
                next_process->state = RUNNING;
                scheduler.current_process = next_process;
                printf(" [CPU] Atendido P%d(rem=%d, q=%d)\n", next_process->pid, next_process->remaining_time, scheduler.quantum_remaining);
            } else { printf(" [CPU] CPU inactivo(sin procesos en READY)\n"); }
        }

// --------------------------------------

        SchedulerResults sch_results = schedulerTick(&scheduler);
        Process *current = scheduler.current_process;

        printf(" [CPU] Ejecutando P%d | rem=%d | q_rem=%d\n", current->pid, current->remaining_time, scheduler.quantum_remaining);

        if(sch_results != SCHEDULER_RUNNING && current != NULL){
            printf(" [TICK] P%d | estado=%s | rem=%d | q_rem=%d\n", current->pid, resultToString(sch_results), current->remaining_time, scheduler.quantum_remaining);
        }

// --------------------------------------

        switch(sch_results){
            case SCHEDULER_RUNNING: { break; }
            case SCHEDULER_NO_PROCESSES: { break; }
            case SCHEDULER_QUANTUM_EXPIRED: {
                printf("[Q] Quantum agotado para P%d, reencolando al final de READY\n", current->pid); 
                schedulerPreempt(&scheduler);
                break;
            }
            case SCHEDULER_PROCESS_FINISHED: {
                Process *done = schedulerTerminateProcess(&scheduler);

                if(done != NULL){
                    printf(" [EXIT] P%d terminado(wait=%d) --> liberando %d KB\n", done->pid, done->waiting_time, done->memory_required);
                    finished_processes++;
                    memoryFree(&memory, done);
                    memoryCondense(&memory);
                    processDestroy(&done);
                    printMemory(&memory);
                }
                break;                        
            }
        }
        system_tick++;
    }

// -------------------------------------------

    printf("\n------------------------------------------------\n");
    printf("\nSimulación terminó en %d ticks. \nContext switches: %d\n", system_tick, scheduler.context_switchs);
    printMemory(&memory);

    deleteQueue(&new_processes);
    schedulerDestroy(&scheduler);
    memoryDestroy(&memory);

    return 0;
}
