#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

#define DEFAULT_SYSTEM_QUANTUM 2
#define MAX_SYSTEM_QUANTUM 5
#define MIN_SPLIT 32
#define MAX_MEMORY_PROCESS 500 // limit for the memory: 4 MB
#define MAX_QUANTUM_PROCESS 10

typedef struct {
    int system_quantum;
    int max_memory_process;
    int max_quantum_process;
} Settings;

Settings defaultSettings();
bool validateSettings(int, int, int);
bool changeSettings(Settings*, int, int, int);

#endif