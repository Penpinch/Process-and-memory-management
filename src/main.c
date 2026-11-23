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
#include "../headers/statistics.h"

#define GENERATION_CHANCE 100

int main() {
    Settings settings = defaultSettings();
    Memory memory;
    Scheduler scheduler;
    ProcessQueue new_processes;
    TerminalControl ctl;
    Statistics stats;
    int system_tick = 0;
    int finished_processes = 0;

    srand((unsigned)time(NULL));
    resetCurrentPid();

    terminalControlInit(&ctl);
    if (!terminalConfigure(&settings)) {
        printf("Configuracion invalida, se usan los valores por defecto.\n");
    }

    memoryInit(&memory, settings);
    schedulerInit(&scheduler, settings.scheduler_quantum);
    queueInit(&new_processes);
    statsInit(&stats, settings.memory_size);

    for (;;) {

        if (rand() % 100 < GENERATION_CHANCE) {
            Process *process = generateProcess(&settings, system_tick);
            if (process != NULL) {
                if (enqueue(&new_processes, process)) {
                    statsRecordProcessGenerated(&stats);
                } else {
                    processDestroy(&process);
                }
            }
        }

        while (!isEmpty(&new_processes)) {
            if (!placeInMemory(&memory, queueFront(&new_processes))) {
                statsRecordProcessRejected(&stats);
                break;
            }
            schedulerAddProcess(&scheduler, dequeue(&new_processes));
        }

        if (scheduler.current_process == NULL) {
            Process *next = schedulerAttend(&scheduler);
            if (next != NULL) {
                next->state = RUNNING;
                scheduler.current_process = next;
            }
        }

        SchedulerResults result = schedulerTick(&scheduler);
        long quantum_used_in_tick = (result != SCHEDULER_NO_PROCESSES) ? 1 : 0;

        switch (result) {
            case SCHEDULER_QUANTUM_EXPIRED:
                schedulerPreempt(&scheduler);
                break;

            case SCHEDULER_PROCESS_FINISHED: {
                Process *done = schedulerTerminateProcess(&scheduler);
                if (done != NULL) {
                    statsRecordProcessCompleted(&stats, 0, done->waiting_time);
                    memoryFree(&memory, done);
                    memoryCondense(&memory);
                    processDestroy(&done);
                    finished_processes++;
                }
                break;
            }

            default:
                break;
        }

        statsRecordCycle(&stats, &memory, quantum_used_in_tick);

        printf("Evento del ciclo: %s\n", resultToString(result));
        printSimulationScreen(system_tick, &memory, &new_processes, &scheduler, &ctl,settings);
        statsPrintLive(&stats);

        if (terminalWaitNextTick(&ctl) == TERMINAL_STOP) { break; }
        system_tick++;
    }

    printf("\n==================================================\n");
    printf("Ciclos ejecutados: %d | Procesos terminados: %d | Cambios de contexto: %d\n",
           system_tick + 1, finished_processes, scheduler.context_switchs);
    printMemory(&memory,settings);
    statsPrintSummary(&stats);

    Process *pending;
    while ((pending = dequeue(&new_processes)) != NULL) {
        processDestroy(&pending);
    }

    memoryDestroy(&memory, settings);
    schedulerDestroy(&scheduler);
    return 0;
}