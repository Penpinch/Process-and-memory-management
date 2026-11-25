#include "../headers/memory.h"
#include <stddef.h>
#include <stdlib.h>

bool memoryInit(Memory* memory, const Settings settings){
    memory->total_kb = settings.memory_size*1024;
    memory->max_blocks = memory->total_kb;
    memory->blocks = malloc((size_t)memory->max_blocks * sizeof(Block));
    
    if (memory->blocks == NULL){
        memory->block_count = 0;
        return false;
    } // if

    memory->blocks[0].process = NULL;
    memory->blocks[0].size = memory->total_kb;
    memory->block_count = 1;
    return true;
}

int memoryFirstFit(const Memory* memory, int process_size){
    for (int i=0; i<memory->block_count; i++)
        if (memory->blocks[i].process == NULL && memory->blocks[i].size >= process_size) return i;

    return -1;
}
int memoryBestFit(const Memory* memory, int process_size){
    int indx = -1;
    for (int i=0; i<memory->block_count; i++)
        if (memory->blocks[i].process == NULL && memory->blocks[i].size >= process_size && (indx == -1 || memory->blocks[i].size < memory->blocks[indx].size))
            indx = i;

    return indx;
}

void insertGap(Memory* memory, int limit_indx){
    for (int i=memory->block_count; i>limit_indx; i--)
        memory->blocks[i] = memory->blocks[i-1]; 
}
bool memoryAllocate(Memory* memory, Process* process, int indx){
    if (process == NULL || indx < 0 || indx >= memory->block_count) return false;
    if (memory->blocks[indx].process != NULL) return false; // not a hole

    int h = memory->blocks[indx].size;
    int p = process->memory_required;
    if (p <= 0 || p > h) return false;

    if (h-p > 0){
        if (memory->block_count >= memory->max_blocks) return false;
        insertGap(memory, indx);
        memory->blocks[indx+1].process = NULL;
        memory->blocks[indx+1].size = h-p;
        memory->block_count++;
    } // if

    memory->blocks[indx].process = process;
    memory->blocks[indx].size = p;
    
    return true;
}

bool memoryFree(Memory* memory, Process* process){
    if (process == NULL) return false;

    for (int i=0; i<memory->block_count; i++)
        if (memory->blocks[i].process == process){
            memory->blocks[i].process = NULL;
            return true;
        } // if
    
    return false;
}
void removeBlock(Memory* memory, int limit_indx){
    memory->block_count--;
    for (int i=limit_indx; i<memory->block_count; i++)
        memory->blocks[i] = memory->blocks[i+1];
}
void memoryCondense(Memory* memory){
    int i=0;
    while (i+1 < memory->block_count){
        if (memory->blocks[i].process == NULL && memory->blocks[i+1].process == NULL){
            memory->blocks[i].size += memory->blocks[i+1].size;
            removeBlock(memory, i+1);
        } else i++;
    } // while
}

void memoryDestroy(Memory* memory){
    for (int i=0; i<memory->block_count; i++)
        processDestroy(&memory->blocks[i].process);

    free(memory->blocks);
    memory->blocks = NULL;
    memory->block_count = 0;
    memory->max_blocks = 0;
}

bool placeInMemory(Memory *memory, Process *process, FitAlgorithm algorithm){
    int indx = (algorithm == BEST_FIT) ? memoryBestFit(memory, process->memory_required) : memoryFirstFit(memory, process->memory_required);
    return memoryAllocate(memory, process, indx);
}
