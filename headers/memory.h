#ifndef MEMORY_H
#define MEMORY_H

#include "../headers/process.h"
#include <stdbool.h>

typedef struct {
    Process *process; // NULL = hole
    int size; // in KB
} Block;

typedef struct {
    int total_kb; // memory_size * 1024
    int max_blocks; // 2 * (TOTAL_MEMORY / MIN_SPLIT) + 1
    Block *blocks;
    int block_count;
} Memory;

bool memoryInit(Memory*, const Settings);

int memoryFirstFit(const Memory*, int);
int memoryBestFit(const Memory*, int);

void insertGap(Memory*, int);
bool memoryAllocate(Memory*, Process*, int);

bool memoryFree(Memory*, Process*); // doesn't destroy the process
void removeBlock(Memory*, int);
void memoryCondense(Memory*);

void memoryDestroy(Memory*);

bool placeInMemory(Memory *, Process *, FitAlgorithm);

#endif
