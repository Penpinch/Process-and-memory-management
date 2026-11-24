#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "../headers/settings.h"
#include "../headers/process.h"
#include "../headers/queue.h"
#include "../headers/scheduler.h"
#include "../headers/memory.h"
#include "../headers/terminal.h"
#include "../headers/stadistics.h"

#define GENERATION_CHANCE 100

void printProc(const Process *p){ printf("[%d, %d, %d]", p->pid, p->memory_required, p->remaining_time); }

int main(){
    setvbuf(stdout, NULL, _IOFBF, 1 << 16);

    Settings settings = defaultSettings();
    Memory memory;
    Scheduler scheduler;
    ProcessQueue new_processes;
    TerminalControl ctl;
    Statistics stats;
    int system_tick = 1;
    int finished_processes = 0;

    srand((unsigned)time(NULL));
    resetCurrentPid();

    terminalControlInit(&ctl);
    if (!terminalConfigure(&settings)) {
        printf("Invalid configuration: default values are being used\n");
    }

    if (!memoryInit(&memory, settings)) {
        printf("There is not enough memory to start the simulation\n");
        return 1;
    }
    schedulerInit(&scheduler, settings.system_quantum);
    queueInit(&new_processes);
    statsInit(&stats, (size_t)memory.total_kb);

    for (;;) {
        if(rand() % 100 < GENERATION_CHANCE){
            Process *process = generateProcess(&settings, system_tick);
            if(process != NULL){
                if(enqueue(&new_processes, process)){
                    statsRecordProcessGenerated(&stats);
                    printf("\n\nProcess entry: ");
                    printProc(process);
                    printf("\n");
                } else { processDestroy(&process); }
            }
        }

        bool allocated_any = false;
        while(!isEmpty(&new_processes)){
            Process *front = queueFront(&new_processes);
            if(!placeInMemory(&memory, front, settings.algorithm)){
                statsRecordProcessRejected(&stats);
                printf("Not enough space for ");
                printProc(front);
                printf("\n");
                break;
            }
            Process *accepted = dequeue(&new_processes);
            schedulerAddProcess(&scheduler, accepted);
            allocated_any = true;
        }
        if(allocated_any){ printMemory(&memory); }

        if(scheduler.current_process == NULL){
            Process *next = schedulerAttend(&scheduler);
            if(next != NULL){
                next->state = RUNNING;
                scheduler.current_process = next;
                printf("Attending process:\n");
                printProc(next);
                printf("\n");
            } 
        }

        SchedulerResults result = schedulerTick(&scheduler);
        long quantum_used_in_tick = (result != SCHEDULER_NO_PROCESSES) ? 1 : 0;

        if(result == SCHEDULER_QUANTUM_EXPIRED){ schedulerPreempt(&scheduler); }

        printProcessQueue(&scheduler.ready_queue, "RR list");

        if(result == SCHEDULER_PROCESS_FINISHED){
            Process *done = schedulerTerminateProcess(&scheduler);
            
            if(done != NULL){
                processSetFinish(done, system_tick + 1);
                statsRecordProcessCompleted(&stats, done->execution_time, done->waiting_time);
                printf("Deloading process");
                printProc(done);
                printf("\n");
                memoryFree(&memory, done);
                printMemory(&memory);

                printf("Condencing holes\n");
                memoryCondense(&memory);
                printMemory(&memory);
                processDestroy(&done);
                finished_processes++;
            }
        } else { printMemory(&memory); }

        statsRecordCycle(&stats, &memory, quantum_used_in_tick);

        if (terminalWaitNextTick(&ctl) == TERMINAL_STOP) { break; }

        system_tick++;
    }

    printf("\n==================================================\n");
    statsPrintSummary(&stats);

    Process *pending;
    while((pending = dequeue(&new_processes)) != NULL){ processDestroy(&pending); }

    memoryDestroy(&memory);
    schedulerDestroy(&scheduler);
    return 0;
}
