#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>

#include "../headers/settings.h"
#include "../headers/queue.h"
#include "../headers/scheduler.h"
#include "../headers/memory.h"

#define TERMINAL_DELAY_DEFAULT 500   
#define TERMINAL_DELAY_STEP 100   
#define TERMINAL_DELAY_MAX 2000
#define TERMINAL_QUEUE_PREVIEW 10 // processes shown per queue

typedef struct {
    int delay_ms;     // simulation speed
    bool step_mode;   // true: advances one cycle for each Enter
    bool refresh_mode; // treu: redraw in the same place, false: preserve history
} TerminalControl;

typedef enum { TERMINAL_CONTINUE, TERMINAL_STOP } TerminalAction;

const char *resultToString(SchedulerResults sch_results);

void printMemory(const Memory *m);
void printProcessQueue(const ProcessQueue *queue, const char *title);
void printCurrentProcess(const Scheduler *scheduler);
void printSimulationScreen(int tick, const char *event, const Memory *m, const ProcessQueue *new_processes, const Scheduler *scheduler, const TerminalControl *ctl);

void terminalControlInit(TerminalControl *ctl);
TerminalAction terminalWaitNextTick(TerminalControl *ctl);

bool terminalConfigure(Settings *settings);

#endif