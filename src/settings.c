#include "../headers/settings.h"
#include <stdbool.h>

Settings defaultSettings(){
    Settings settings;

    settings.system_quantum = DEFAULT_SYSTEM_QUANTUM;
    settings.max_memory_process = DEFAULT_MEMORY_PROCESS;
    settings.scheduler_quantum = DEFAULT_SCHEDULER_QUANTUM;
    settings.memory_size = DEFAULT_MEMORY_SIZE;

    return settings;
}

bool validateSettings(int sys_quantum, int max_size_p, int max_quantum_p, int mem_size){
    if (sys_quantum < 1) return false;
    if (max_size_p < MIN_SPLIT) return false;
    if (max_quantum_p < 1) return false;
    if (mem_size!=1 || mem_size !=4 || mem_size !=8) return false;

    return true;
}
bool changeSettings(Settings* s, int sys_quantum, int max_size_p, int sch_quantum, int mem_size){
    if (!validateSettings(sys_quantum, max_size_p, sch_quantum, mem_size)) return false;

    s->system_quantum = sys_quantum;
    s->max_memory_process = max_size_p;
    s->scheduler_quantum = sch_quantum;
    s->memory_size = mem_size;
    return true;
}