#ifndef MEMORY_H
#define MEMORY_H

#include "../headers/process.h"
#include <stdbool.h>

typedef enum {ONE_MB = 1024, FOUR_MB = 4096, EIGHT_MB = 8192} TOTAL_MEMORY ;
typedef enum {BLOCKS_ONE_MB = 65, BLOCKS_FOUR_MB = 257, BLOCKS_EIGHT_MB = 513} MAX_BLOCKS;

typedef struct {
    Process *process; // NULL = hole
    int size; // in KB
} Block;

typedef struct {
    int max_blocks; // 2 * (TOTAL_MEMORY / MIN_SPLIT) + 1
    Block *blocks;
    int block_count;
} Memory;

void memoryInit(Memory*, const Settings);

int memoryFirstFit(const Memory*, int);
int memoryBestFit(const Memory*, int);

void insertGap(Memory*, int);
bool memoryAllocate(Memory*, Process*, int);

bool memoryFree(Memory*, Process*); // doesn't destroy the process
void removeBlock(Memory*, int);
void memoryCondense(Memory*);

void memoryDestroy(Memory*, const Settings);

bool placeInMemory(Memory *memory, Process *process);

#endif
