#include "../headers/settings.h"
#include <stdbool.h>

Settings defaultSettings(){
    Settings settings;

    settings.system_quantum = DEFAULT_SYSTEM_QUANTUM;
    settings.max_memory_process = MAX_MEMORY_PROCESS;
    settings.scheduler_quantum = DEFAULT_SCHEDULER_QUANTUM;

    return settings;
}

bool validateSettings(int sys_quantum, int max_size_p, int max_quantum_p){
    if (sys_quantum < 1 || sys_quantum > MAX_SYSTEM_QUANTUM) return false;
    if (max_size_p < MIN_SPLIT || max_size_p > MAX_MEMORY_PROCESS) return false;
    if (max_quantum_p < 1 || max_quantum_p > MAX_SCHEDULER_QUANTUM) return false;
    
    return true;
}
bool changeSettings(Settings* s, int sys_quantum, int max_size_p, int sch_quantum){
    if (!validateSettings(sys_quantum, max_size_p, sch_quantum)) return false;

    s->system_quantum = sys_quantum;
    s->max_memory_process = max_size_p;
    s->scheduler_quantum = sch_quantum;
    return true;
}

/* -- TEST --

#include <stdio.h>
#include <stdlib.h>

void showSettings(const Settings s){
    printf("\n");
    printf("System quantum: %d \n", s.system_quantum);
    printf("Maximum memory (process): %d \n", s.max_memory_process);
    printf("Maximum quantum (process): %d \n", s.max_quantum_process);
    printf("\n");
}

int main(){
    Settings s = defaultSettings();
    
    printf("\nDEFAULT");
    showSettings(s); // valid = default
    
    if (changeSettings(&s, 1, 250, 5)) // valid
        showSettings(s);
    else printf("\n Invalid data entered, changes not saved \n");
    if (changeSettings(&s, 0, 500, 10)) // invalid
        showSettings(s);
    else printf("\n Invalid data entered, changes not saved \n");
    if (changeSettings(&s, 5, 750, 10)) // invalid
        showSettings(s);
    else printf("\n Invalid data entered, changes not saved \n");
    if (changeSettings(&s, 5, 300, 10)) // valid
        showSettings(s);
    else printf("\n Invalid data entered, changes not saved \n");
    if (changeSettings(&s, 2, 400, 20)) // invalid
        showSettings(s);
    else printf("\n Invalid data entered, changes not saved \n");

    return 0;
}
*/
