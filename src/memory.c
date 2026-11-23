#include "../headers/memory.h"
#include <stddef.h>
#include <stdlib.h>

void memoryInit(Memory* memory, const Settings settings){
    memory->blocks = malloc(sizeof(int));
    switch(settings.memory_size){
        case 1: memory->blocks[BLOCKS_ONE_MB];
                memory->blocks[0].size = ONE_MB;
                break;
        case 4: memory->blocks[BLOCKS_FOUR_MB];
                memory->blocks[0].size = FOUR_MB;
                break;
        case 8: memory->blocks[BLOCKS_EIGHT_MB];
                memory->blocks[0].size = EIGHT_MB;
                break;
    } // switch
    
    memory->blocks[0].process = NULL;
    memory->block_count = 1;
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
    if (indx == -1) return false;

    int h = memory->blocks[indx].size;
    int p = process->memory_required;

    if (h-p > 0){
        insertGap(memory, indx);
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

void memoryDestroy(Memory* memory, const Settings settings){
    for (int i=0; i<memory->block_count; i++)
        processDestroy(&memory->blocks[i].process);
    memoryInit(memory, settings);
}

bool placeInMemory(Memory *memory, Process *process){
    return memoryAllocate(memory, process, memoryFirstFit(memory, process->memory_required));
}
