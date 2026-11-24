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

int main() {
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
            if (!placeInMemory(&memory, queueFront(&new_processes), settings.algorithm)) {
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
                    processSetFinish(done, system_tick + 1); // also computes waiting_time
                    statsRecordProcessCompleted(&stats, done->execution_time, done->waiting_time);
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

        printSimulationScreen(system_tick, resultToString(result), &memory, &new_processes, &scheduler, &ctl);
        statsPrintLive(&stats);

        if (terminalWaitNextTick(&ctl) == TERMINAL_STOP) { break; }
        system_tick++;
    }

    printf("\n==================================================\n");
    printf("Executed cycles: %d | Completed processes: %d | Context switches: %d\n",
           system_tick, finished_processes, scheduler.context_switchs);
    printMemory(&memory);
    statsPrintSummary(&stats);

    Process *pending;
    while ((pending = dequeue(&new_processes)) != NULL) {
        processDestroy(&pending);
    }

    memoryDestroy(&memory);
    schedulerDestroy(&scheduler);
    return 0;
}