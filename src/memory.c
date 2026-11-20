#include "../headers/memory.h"
#include <stddef.h>

void memoryInit(Memory* memory){
    memory->blocks[0].process = NULL;
    memory->blocks[0].size = TOTAL_MEMORY;
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

void memoryDestroy(Memory* memory){
    for (int i=0; i<memory->block_count; i++)
        processDestroy(&memory->blocks[i].process);
    memoryInit(memory);
}

/* -- TEST -- */

#include <stdio.h>

static int failures = 0;

static void check(const char *name, bool ok){
    printf("  [%s] %s\n", ok ? "OK" : "FAIL", name);
    if (!ok) failures++;
}

static void printMemory(const Memory *m){
    for (int i = 0; i < m->block_count; i++){
        if (m->blocks[i].process == NULL)
            printf("[0,%d,0]", m->blocks[i].size);
        else
            printf("[%d,%d,%d]", m->blocks[i].process->pid, m->blocks[i].size,
                   m->blocks[i].process->remaining_time);
    }
    printf("\n");
}

/* True if the block list is a valid memory layout */
static bool isConsistent(const Memory *m){
    if (m->block_count < 1 || m->block_count > MAX_BLOCKS) return false;

    int total = 0;
    for (int i = 0; i < m->block_count; i++){
        const Block *b = &m->blocks[i];
        if (b->size <= 0) return false;                                   // empty block
        if (i > 0 && b->process == NULL && m->blocks[i-1].process == NULL) // two adjacent holes
            return false;
        if (b->process != NULL && b->size != b->process->memory_required) // wrong split
            return false;
        total += b->size;
    }
    return total == TOTAL_MEMORY;
}

static Process *newProc(const Settings *s, int size){
    Process *p = generateProcess(s, 0);
    if (p) p->memory_required = size; // force the size so results are predictable
    return p;
}

static bool place(Memory *m, Process *p){
    return memoryAllocate(m, p, memoryFirstFit(m, p->memory_required));
}

int main(){
    Settings s = defaultSettings();
    Memory m;
    resetCurrentPid();
    memoryInit(&m);

    printf("1. First fit, split and exact fit\n");
    Process *a = newProc(&s, 1000), *b = newProc(&s, 100), *c = newProc(&s, 2000);
    Process *d = newProc(&s, 200),  *e = newProc(&s, 796);
    place(&m, a); place(&m, b); place(&m, c); place(&m, d);
    printMemory(&m);
    check("5 blocks (4 processes + final hole)", m.block_count == 5);
    place(&m, e);
    printMemory(&m);
    check("exact fit: no new block", m.block_count == 5);
    check("consistent", isConsistent(&m));

    printf("2. Free blocks that have no hole neighbors\n");
    memoryFree(&m, a); memoryFree(&m, d);
    printMemory(&m);
    check("consistent (isolated holes)", isConsistent(&m));
    memoryCondense(&m);
    check("condense merges nothing", m.block_count == 5);

    printf("3. First fit vs best fit (holes: 1000 at block 0, 200 at block 3)\n");
    check("150: first fit -> block 0", memoryFirstFit(&m, 150) == 0);
    check("150: best fit -> block 3", memoryBestFit(&m, 150) == 3);
    check("200: best fit exact -> block 3", memoryBestFit(&m, 200) == 3);
    check("500: both -> block 0", memoryFirstFit(&m, 500) == 0 && memoryBestFit(&m, 500) == 0);
    check("1500: neither fits", memoryFirstFit(&m, 1500) == -1 && memoryBestFit(&m, 1500) == -1);

    Process *q = newProc(&s, 150);
    check("allocate q with best fit", memoryAllocate(&m, q, memoryBestFit(&m, 150)));
    printMemory(&m);
    check("q is at block 3", m.blocks[3].process == q);
    check("leftover hole of 50 at block 4", m.blocks[4].process == NULL && m.blocks[4].size == 50);
    check("6 blocks and consistent", m.block_count == 6 && isConsistent(&m));
    check("allocate with index -1 fails", !memoryAllocate(&m, q, -1));

    printf("4. Merge several holes at once\n");
    memoryFree(&m, b); memoryFree(&m, c); memoryFree(&m, q);
    printMemory(&m);
    check("adjacent holes are detected as inconsistent", !isConsistent(&m));
    memoryCondense(&m);
    printMemory(&m);
    check("one hole of 3300 + process e", m.block_count == 2 && m.blocks[0].size == 3300);
    check("consistent", isConsistent(&m));
    memoryCondense(&m);
    check("second condense changes nothing", m.block_count == 2);

    printf("5. Invalid memoryFree calls\n");
    Process *ghost = newProc(&s, 100); // never placed in memory
    check("process not in memory", !memoryFree(&m, ghost));
    check("NULL", !memoryFree(&m, NULL));
    check("already freed process", !memoryFree(&m, b));

    printf("6. memoryDestroy (e is still in memory)\n");
    processDestroy(&a); processDestroy(&b); processDestroy(&c);
    processDestroy(&d); processDestroy(&q); processDestroy(&ghost);
    memoryDestroy(&m);   // destroys e
    e = NULL;            // e is dangling now, do not use it
    check("one hole of 4096",
          m.block_count == 1 && m.blocks[0].size == TOTAL_MEMORY && m.blocks[0].process == NULL);

    printf("7. 128 processes of 32 KB\n");
    Process *many[128];
    for (int i = 0; i < 128; i++){ many[i] = newProc(&s, 32); place(&m, many[i]); }
    check("full memory: 128 blocks", m.block_count == 128 && isConsistent(&m));
    check("another 32 KB does not fit", memoryFirstFit(&m, 32) == -1 && memoryBestFit(&m, 32) == -1);
    for (int i = 1; i < 128; i += 2) memoryFree(&m, many[i]);
    memoryCondense(&m);
    check("alternating holes are not merged", m.block_count == 128 && isConsistent(&m));
    for (int i = 0; i < 128; i += 2) memoryFree(&m, many[i]);
    memoryCondense(&m);
    check("all free: one hole of 4096", m.block_count == 1 && m.blocks[0].size == TOTAL_MEMORY);
    for (int i = 0; i < 128; i++) processDestroy(&many[i]);

    printf("8. Many tiny holes (1 KB) between processes\n");
    Process *big[124], *spacer[124];
    for (int i = 0; i < 124; i++){
        big[i] = newProc(&s, 32);
        spacer[i] = newProc(&s, 1);
        place(&m, big[i]);
        place(&m, spacer[i]);
    }
    check("248 blocks + final hole of 4 KB", m.block_count == 249 && isConsistent(&m));
    for (int i = 0; i < 124; i++) memoryFree(&m, spacer[i]);
    memoryCondense(&m);
    check("1 KB holes stay separated", m.block_count == 249 && isConsistent(&m));
    check("1 KB request: first fit takes the first hole", memoryFirstFit(&m, 1) == 1);
    int best = memoryBestFit(&m, 1);
    check("1 KB request: best fit takes a 1 KB hole", best != -1 && m.blocks[best].size == 1);
    check("2 KB request: only the final hole fits",
          memoryFirstFit(&m, 2) == 248 && memoryBestFit(&m, 2) == 248);
    for (int i = 0; i < 124; i++) memoryFree(&m, big[i]);
    memoryCondense(&m);
    check("all free: one hole of 4096", m.block_count == 1 && m.blocks[0].size == TOTAL_MEMORY);
    for (int i = 0; i < 124; i++){ processDestroy(&big[i]); processDestroy(&spacer[i]); }

    printf("\n%s (%d failures)\n", failures ? "SOME TESTS FAILED" : "ALL TESTS PASSED", failures);
    return failures != 0;
}