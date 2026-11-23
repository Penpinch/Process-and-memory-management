#define _DEFAULT_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/terminal.h"

#define KEY_POLL_MS 20

//Lectura de teclas y pausas.
#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
  #define SLEEP_MS(ms) Sleep(ms)
  static bool keyPressed(void){ return _kbhit() != 0; }
  static int  readKey(void){ return _getch(); }
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <signal.h>

    #define SLEEP_MS(ms) usleep((useconds_t)(ms) * 1000)

    static struct termios g_orig_termios;
    static int  g_orig_flags = -1;
    static bool g_raw_active = false;
    static int  g_peek = -1;

    static void terminalDisableRawMode(){
        if(!g_raw_active){ return; }
        tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);

        if(g_orig_flags != -1){ fcntl(STDIN_FILENO, F_SETFL, g_orig_flags); }
        g_raw_active = false;
    }

    static void onSignal(int sig){
        terminalDisableRawMode();
        signal(sig, SIG_DFL);
        raise(sig);
    }

    static void terminalEnableRawMode(){
        if(g_raw_active){ return; }
        if(tcgetattr(STDIN_FILENO, &g_orig_termios) == -1){ return; }

        struct termios raw = g_orig_termios;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        if(tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0){ return; }

        g_orig_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        if(g_orig_flags != -1){ fcntl(STDIN_FILENO, F_SETFL, g_orig_flags | O_NONBLOCK); }

        g_raw_active = true;
        atexit(terminalDisableRawMode);
        signal(SIGINT, onSignal);
        signal(SIGTERM, onSignal);
    }

    static bool keyPressed(){
        if(g_peek != -1){ return true; }
        unsigned char ch;
        if(read(STDIN_FILENO, &ch, 1) == 1){ g_peek = ch; return true; }
        return false;
    }

    static int readKey(){
        if(g_peek != -1){ int k = g_peek; g_peek = -1; return k; }

        unsigned char ch;
        for(ssize_t n = read(STDIN_FILENO, &ch, 1); n != 1; n = read(STDIN_FILENO, &ch, 1)){
            SLEEP_MS(KEY_POLL_MS);
        }
        return (int)ch;
    }
#endif

const char *resultToString(SchedulerResults sch_results){
    switch(sch_results){
        case SCHEDULER_NO_PROCESSES: return "No processes";
        case SCHEDULER_RUNNING: return "Running";
        case SCHEDULER_QUANTUM_EXPIRED: return "Quantum expired";
        case SCHEDULER_PROCESS_FINISHED: return "Process finished";
        default: return "Unknowd";
    }
}

// Visualizacion

void printMemory(const Memory *m){
    printf(" Memory(%d blocks, %d total): ", m->block_count, TOTAL_MEMORY);
    for(int i = 0; i < m->block_count; i++){
        if(m->blocks[i].process == NULL){
            printf("[0,%d,0]", m->blocks[i].size);
        } else {
            printf("[%d,%d,%d]", m->blocks[i].process->pid, m->blocks[i].size, m->blocks[i].process->remaining_time);
        }
    }
    printf("\n");
}

void printProcessQueue(const ProcessQueue *queue, const char *title){
    printf("%s (%d): ", title, queue->size);
    if(isEmpty(queue)){ printf("vacia"); }
    for(const Node *n = queue->front; n != NULL; n = n->next){
        printf("[%d,%d,%d] ", n->process->pid, n->process->memory_required, n->process->remaining_time);
    }
    printf("\n");
}

void printCurrentProcess(const Scheduler *scheduler){
    const Process *p = scheduler->current_process;

    if(p == NULL){
        printf("CPU: inactiva (sin proceso en ejecución)\n");
        return;
    }
    printf("CPU: [%d,%d,%d] | quantum restante: %d de %d | cambios de contexto: %d\n",
           p->pid, p->memory_required, p->remaining_time,
           scheduler->quantum_remaining, scheduler->quantum,
           scheduler->context_switchs);
}

void printSimulationScreen(int tick, const Memory *m, const ProcessQueue *new_processes, const Scheduler *scheduler, const TerminalControl *ctl){
    int used = 0;
    for(int i = 0; i < m->block_count; i++){
        if(m->blocks[i].process != NULL){ used += m->blocks[i].size; }
    }

    printf("\n -----CICLO----- %d \n", tick);
    printf("Velocidad: %d ms | Modo: %s\n", ctl->delay_ms,
           ctl->step_mode ? "paso a paso" : "automatico");

    printf("--- Memoria ---\n");
    printMemory(m);
    printf("Ocupada: %d KB (%.1f%%) | Libre: %d KB\n",
           used, 100.0 * used / TOTAL_MEMORY, TOTAL_MEMORY - used);

    printf("--- Procesos ---\n");
    printProcessQueue(new_processes, "NEW ");
    printProcessQueue(&scheduler->ready_queue, "RR  ");
    printCurrentProcess(scheduler);
    printf("\n");
}

//Control

void terminalControlInit(TerminalControl *ctl){
    ctl->delay_ms = TERMINAL_DELAY_DEFAULT;
    ctl->step_mode = false;
}

static bool waitResume(void){
    printf("\n[PAUSA] p: continuar | q: salir\n");
    for(;;){
        int k = readKey();
        if(k == 'p' || k == 'P'){ return true; }
        if(k == 'q' || k == 'Q'){ return false; }
    }
}

/*
Teclas:
  Enter (modo paso a paso) : siguiente ciclo
  s : alterna entre paso a paso y automatico
  + : mas rapido    - : mas lento
  p : pausa         q : terminar simulacion
*/
TerminalAction terminalWaitNextTick(TerminalControl *ctl){
    terminalEnableRawMode();

    if(ctl->step_mode){
        printf("[Enter] siguiente ciclo | s: automatico | q: salir > ");
        fflush(stdout);
        for(;;){
            int k = readKey();
            if(k == 'q' || k == 'Q'){ return TERMINAL_STOP; }
            if(k == 's' || k == 'S'){ ctl->step_mode = false; break; }
            if(k == '\n' || k == '\r'){ return TERMINAL_CONTINUE; }
        }
    }

    int waited = 0;
    for(;;){
        if(keyPressed()){
            switch(readKey()){
                case 'q': case 'Q':
                    return TERMINAL_STOP;
                case 's': case 'S':
                    ctl->step_mode = true;
                    return TERMINAL_CONTINUE;
                case 'p': case 'P':
                    if(!waitResume()){ return TERMINAL_STOP; }
                    break;
                case '+':
                    ctl->delay_ms -= TERMINAL_DELAY_STEP;
                    if(ctl->delay_ms < 0){ ctl->delay_ms = 0; }
                    break;
                case '-':
                    ctl->delay_ms += TERMINAL_DELAY_STEP;
                    if(ctl->delay_ms > TERMINAL_DELAY_MAX){ ctl->delay_ms = TERMINAL_DELAY_MAX; }
                    break;
                default: break;
            }
        }
        if(waited >= ctl->delay_ms){ break; }
        SLEEP_MS(KEY_POLL_MS);
        waited += KEY_POLL_MS;
    }
    return TERMINAL_CONTINUE;
}

//Settings

static int askInt(const char *label, int current, int min, int max){
    char line[64];

    for(;;){
        printf("%s [%d-%d] (actual %d): ", label, min, max, current);
        if(fgets(line, sizeof line, stdin) == NULL){ return current; }
        if(line[0] == '\n'){ return current; }

        int value;
        if(sscanf(line, "%d", &value) == 1 && value >= min && value <= max){
            return value;
        }
        printf("  Valor invalido. \n");
    }
}

bool terminalConfigure(Settings *settings){
    printf("\n CONFIGURACION \n");
    printf("(Enter conserva el valor actual)\n");

    int sys_q = askInt("Cuanto del sistema, quantum de creacion",
                       settings->system_quantum, 1, MAX_SYSTEM_QUANTUM);
    int max_p = askInt("Tamano maximo de memoria por proceso (KB)",
                       settings->max_memory_process, MIN_SPLIT, MAX_MEMORY_PROCESS);
    int sch_q = askInt("Cuanto maximo de procesamiento por proceso (Round Robin)",
                       settings->scheduler_quantum, 1, MAX_SCHEDULER_QUANTUM);
    return changeSettings(settings, sys_q, max_p, sch_q);
}
