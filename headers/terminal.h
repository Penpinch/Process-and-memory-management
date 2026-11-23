/*
#ifndef TERMINAL_H
#define TERMINAL_H

#include "../headers/scheduler.h"
#include "../headers/memory.h"

const char *resultToString(SchedulerResults sch_results);

void printMemory(const Memory *m);

#endif
*/

#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>
#include "../headers/scheduler.h"
#include "../headers/memory.h"
#include "../headers/queue.h"
#include "../headers/settings.h"

#define TERMINAL_DELAY_DEFAULT 500   // ms de espera entre ciclos
#define TERMINAL_DELAY_STEP    100   // incremento al usar + y -
#define TERMINAL_DELAY_MAX     2000  // ms máximos entre ciclos

typedef struct {
    int delay_ms;     // velocidad de la simulación
    bool step_mode;   // true: avanza un ciclo por cada Enter
} TerminalControl;

typedef enum {
    TERMINAL_CONTINUE,
    TERMINAL_STOP     // el usuario pidió terminar la simulación
} TerminalAction;

const char *resultToString(SchedulerResults sch_results);

void printMemory(const Memory *m);
void printProcessQueue(const ProcessQueue *queue, const char *title);
void printCurrentProcess(const Scheduler *scheduler);
void printSimulationScreen(int tick, const Memory *m,
                           const ProcessQueue *new_processes,
                           const Scheduler *scheduler,
                           const TerminalControl *ctl);

void terminalControlInit(TerminalControl *ctl);
TerminalAction terminalWaitNextTick(TerminalControl *ctl);

bool terminalConfigure(Settings *settings);

#endif
