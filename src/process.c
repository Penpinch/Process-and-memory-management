#include "../headers/process.h"
#include "../headers/settings.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

Process generateProcess(const Settings *s, int current_time){
    static int current_pid = 1;
    Process process;

    // PID
    process.pid = current_pid;
    current_pid++;
    // MEMORY
    process.memory_required = MIN_SPLIT + rand() % (s->max_memory_process - MIN_SPLIT + 1);
    // EXECUTION TIME
    process.execution_time = 1 + rand() % s->max_quantum_process;
    // REMAINING TIME
    process.remaining_time = process.execution_time;
    // ARRIVAL TIME
    process.arrival_time = current_time;
    // WAITING TIME
    process.waiting_time = 0;
    // FINISH TIME
    process.finish_time = 0;
    // STATE
    process.state = NEW;

    return process;
}

bool processFinished(const Process *p) { return (p->remaining_time <= 0); }
int processRun(Process *p, const Settings *s){
    p->state = RUNNING;
    if (p->remaining_time - s->system_quantum < 0){
        int run_time = p->remaining_time;
        p->remaining_time = 0;
        return run_time;
    } // if

    p->remaining_time -= s->system_quantum;
    return s->system_quantum;
}
void processSetFinish(Process *p, int time) { 
    p->finish_time = time;
    p->waiting_time = p->finish_time - p->arrival_time - p->execution_time; 
    p->state = TERMINATED;
}

/* -- TEST --
#include <stdio.h>

void printProcess(const Process *p){
    printf("[%d,%d,%d]\n", p->pid, p->memory_required, p->remaining_time);
}

int main(){
    int t=10, actual_time = 0;
    Settings s = defaultSettings();
    srand(time(NULL));

    printf(" --- PROCESSES ---\n");
    while (t--){
        Process p = generateProcess(&s, actual_time);
        printProcess(&p);
        actual_time += p.execution_time;
    } // while

    Process p = generateProcess(&s, actual_time);

    printProcess(&p);
    printf("Time:");
    while (!processFinished(&p)){
        int time = processRun(&p, &s);
        printf("  %d", time);
        actual_time += time;
    } // while
    processSetFinish(&p, actual_time);
    printf("\nFinish time: %d\n", p.finish_time);
    printf("Waiting time: %d\n", p.waiting_time);

    return 0;
}
*/
