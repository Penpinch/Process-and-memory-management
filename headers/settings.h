#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

#define MIN_SPLIT 32
#define DEFAULT_SYSTEM_QUANTUM 2
#define DEFAULT_MEMORY_PROCESS 500
#define DEFAULT_SCHEDULER_QUANTUM 4
#define DEFAULT_MEMORY_SIZE 4

typedef struct {
    int system_quantum;
    int max_top_process;
    int max_memory_process;
    int scheduler_quantum;
    int memory_size;
} Settings;

Settings defaultSettings();
bool validateSettings(int, int, int, int);
bool changeSettings(Settings*, int, int, int, int);

#endif
