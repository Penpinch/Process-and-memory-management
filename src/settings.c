#include "../headers/settings.h"
#include <stdbool.h>

Settings defaultSettings(){
    Settings settings;

    settings.system_quantum = DEFAULT_SYSTEM_QUANTUM;
    settings.max_memory_process = DEFAULT_MEMORY_PROCESS;
    settings.max_quantum_process = DEFAULT_MAX_QUANTUM_PROCESS;
    settings.memory_size = DEFAULT_MEMORY_SIZE;
    settings.algorithm = FIRST_FIT;

    return settings;
}

bool validateSettings(int sys_quantum, int max_size_p, int max_quantum_p, int mem_size){
    if (sys_quantum < 1) return false;
    if (max_quantum_p < 1) return false;
    if (mem_size != 1 && mem_size != 4 && mem_size != 8) return false;
    if (max_size_p < 1 || max_size_p > mem_size * 1024) return false; // a process can't be bigger than the memory

    return true;
}
bool changeSettings(Settings* s, int sys_quantum, int max_size_p, int max_quantum_p, int mem_size){
    if (!validateSettings(sys_quantum, max_size_p, max_quantum_p, mem_size)) return false;

    s->system_quantum = sys_quantum;
    s->max_memory_process = max_size_p;
    s->max_quantum_process = max_quantum_p;
    s->memory_size = mem_size;
    return true;
}