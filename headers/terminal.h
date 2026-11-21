#ifndef TERMINAL_H
#define TERMINAL_H

#include "../headers/scheduler.h"
#include "../headers/memory.h"

const char *resultToString(SchedulerResults sch_results);

void printMemory(const Memory *m);

#endif
