#include "../headers/settings.h"
#include <stdbool.h>

Settings defaultSettings(){
    Settings settings;

    settings.system_quantum = DEFAULT_SYSTEM_QUANTUM;
    settings.max_memory_process = MAX_MEMORY_PROCESS;
    settings.max_quantum_process = MAX_QUANTUM_PROCESS;

    return settings;
}

bool validateSettings(int sys_quantum, int max_size_p, int max_quantum_p){
    if (sys_quantum < 1 || sys_quantum > MAX_SYSTEM_QUANTUM) return false;
    if (max_size_p < MIN_SPLIT || max_size_p > MAX_MEMORY_PROCESS) return false;
    if (max_quantum_p < 1 || max_quantum_p > MAX_QUANTUM_PROCESS) return false;
    
    return true;
}
bool changeSettings(Settings* s, int sys_quantum, int max_size_p, int max_quantum_p){
    if (!validateSettings(sys_quantum, max_size_p, max_quantum_p)) return false;

    s->system_quantum = sys_quantum;
    s->max_memory_process = max_size_p;
    s->max_quantum_process = max_quantum_p;
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