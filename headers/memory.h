#ifndef MEMORY_H
#define MEMORY_H

#include "../headers/process.h"
#include <stdbool.h>

#define TOTAL_MEMORY 4096
#define MAX_BLOCKS 257 // 2 * (TOTAL_MEMORY / MIN_SPLIT) + 1

typedef struct {
    Process *process; // NULL = hole
    int size; // in KB
} Block;

typedef struct {
    Block blocks[MAX_BLOCKS];
    int block_count;
} Memory;

void memoryInit(Memory*);

int memoryFirstFit(const Memory*, int);
int memoryBestFit(const Memory*, int);

void insertGap(Memory*, int);
bool memoryAllocate(Memory*, Process*, int);

bool memoryFree(Memory*, Process*); // doesn't destroy the process
void removeBlock(Memory*, int);
void memoryCondense(Memory*);

void memoryDestroy(Memory*);

#endif