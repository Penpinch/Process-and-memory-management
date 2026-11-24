#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

#define DEFAULT_SYSTEM_QUANTUM 2
#define DEFAULT_MEMORY_PROCESS 500
#define DEFAULT_MAX_QUANTUM_PROCESS 10
#define DEFAULT_MEMORY_SIZE 4

typedef enum { FIRST_FIT, BEST_FIT } FitAlgorithm;

typedef struct {
    int system_quantum;
    int max_memory_process;
    int max_quantum_process;
    int memory_size; // in MB
    FitAlgorithm algorithm;
} Settings;

Settings defaultSettings(void);
bool validateSettings(int, int, int, int);
bool changeSettings(Settings*, int, int, int, int);

#endif