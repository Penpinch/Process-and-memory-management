#define _DEFAULT_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../headers/terminal.h"

#define KEY_POLL_MS 20

// Key presses and pauses
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    
    #define SLEEP_MS(ms) Sleep(ms)
    
    static bool keyPressed(void){ return _kbhit() != 0; }
    static int  readKey(void){ return _getch(); }
    static void terminalEnableRawMode(void) {}

    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
    
    static void terminalEnableAnsi(void){
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if(h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode)){
            SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
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
    
    static void terminalEnableAnsi(void){}
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

// Data visualization

void printMemory(const Memory *m){
    printf(" Memory(%d blocks, %d KB total): ", m->block_count, m->total_kb);
    for (int i=0; i<m->block_count; i++){
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

    int shown = 0;
    for(const Node *n = queue->front; n != NULL && shown < TERMINAL_QUEUE_PREVIEW; n = n->next, shown++){
        printf("[%d,%d,%d] ", n->process->pid, n->process->memory_required, n->process->remaining_time);
    }
    if(queue->size > shown){ printf("... +%d more", queue->size - shown); }
    printf("\n");
}

void printCurrentProcess(const Scheduler *scheduler){
    const Process *p = scheduler->current_process;

    if(p == NULL){
        printf("CPU: idle (no process running)\n");
        return;
    }
    printf("CPU: [%d,%d,%d] | remaining quantum: %d de %d | context switches: %d\n",
           p->pid, p->memory_required, p->remaining_time,
           scheduler->quantum_remaining, scheduler->quantum,
           scheduler->context_switchs);
}

static void frameStart(const TerminalControl *ctl){
    if (ctl->refresh_mode) { printf("\033[H\033[J"); } 
    else { printf("\n\n"); }
}

void printSimulationScreen(int tick, const char *event, const Memory *m, const ProcessQueue *new_processes, const Scheduler *scheduler, const TerminalControl *ctl){
    int used = 0;
    for(int i = 0; i < m->block_count; i++){
        if(m->blocks[i].process != NULL){ used += m->blocks[i].size; }
    }

    frameStart(ctl);
    printf("----- CYCLE %d -----\n", tick);
    printf("Event: %s\n", event);
    printf("Speed: %d ms | Mode: %s | Screen: %s\n", ctl->delay_ms,
           ctl->step_mode ? "step-by-step" : "automatic",
           ctl->refresh_mode ? "reflesh" : "record");
    printf("Keys: +/- speed | s step-by-step | r screen | p pause | q exit\n");

    printf("--- Memory ---\n");
    printMemory(m);
    printf("Occupied: %d KB (%.1f%%) | Free: %d KB\n",
           used, 100.0 * used / m->total_kb, m->total_kb - used);

    printf("--- Processes ---\n");
    printProcessQueue(new_processes, "NEW ");
    printProcessQueue(&scheduler->ready_queue, "RR  ");
    printCurrentProcess(scheduler);
    printf("\n");
}

// Control

void terminalControlInit(TerminalControl *ctl){
    ctl->delay_ms = TERMINAL_DELAY_DEFAULT;
    ctl->step_mode = false;
    ctl->refresh_mode = false;
    terminalEnableAnsi();
}

static bool waitResume(void){
    printf("\n[PAUSE] p: continue | q: exit\n");
    fflush(stdout);

    for(;;){
        int k = readKey();
        if(k == 'p' || k == 'P'){ return true; }
        if(k == 'q' || k == 'Q'){ return false; }
    }
}

/*
KEYS:
  Enter (step-by-step mode) : next cycle
  s : toggles between step-by-step and automatic
  + : faster    - : slower
  p : pause         q : end simulation
*/

TerminalAction terminalWaitNextTick(TerminalControl *ctl){
    fflush(stdout);
    terminalEnableRawMode();

    if(ctl->step_mode){
        printf("[Enter] next cycle | s: automatic | q: exit > ");
        fflush(stdout);
        for(;;){
            int k = readKey();
            if(k == 'q' || k == 'Q'){ return TERMINAL_STOP; }
            if(k == 's' || k == 'S'){ ctl->step_mode = false; break; }
            if(k == 'r' || k == 'R'){ ctl->refresh_mode = !ctl->refresh_mode; }
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
                case 'r': case 'R':
                    ctl->refresh_mode = !ctl->refresh_mode;
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

static bool readLine(char *buf, size_t size){
    if(fgets(buf, (int)size, stdin) == NULL){ return false; }
    if(strchr(buf, '\n') == NULL){ // line longer than the buffer: discard the rest
        int c;
        while((c = getchar()) != '\n' && c != EOF){}
    }
    return true;
}

static bool isBlank(const char *line){ return line[0] == '\n' || line[0] == '\r' || line[0] == '\0'; }

static bool parseInt(const char *text, long *out){
    char *end;
    long value = strtol(text, &end, 10);
    if(end == text){ return false; }
    while(*end == ' ' || *end == '\t'){ end++; }
    if(*end != '\n' && *end != '\r' && *end != '\0'){ return false; } // trailing garbage ("12abc")
    *out = value;
    return true;
}

// Integer >= min and <= max. Enter keeps the current value.
static int askInt(const char *label, int current, int min, int max){
    char line[64];

    for(;;){
        if(max == INT_MAX){ printf("%s (minimum %d, current %d): ", label, min, current); }
        else { printf("%s [%d-%d] (current %d): ", label, min, max, current); }
        fflush(stdout);

        if(!readLine(line, sizeof line)){ return current; }
        if(isBlank(line)){ return current; }

        long value;
        if(parseInt(line, &value) && value >= min && value <= max){ return (int)value; }
        printf("  Invalid value: Enter an integer between %d and %d\n", min, max);
    }
}

static int askMemorySize(int current){
    char line[64];

    for(;;){
        printf("Memory size in MB MB (1, 4 u 8) (actual %d): ", current);
        fflush(stdout);

        if(!readLine(line, sizeof line)){ return current; }
        if(isBlank(line)){ return current; }

        long value;
        if(parseInt(line, &value) && (value == 1 || value == 4 || value == 8)){ return (int)value; }
        printf("  Invalid value: only 1, 4, or 8 are permitted\n");
    }
}

static FitAlgorithm askAlgorithm(FitAlgorithm current){
    char line[64];

    for(;;){
        printf("Placement algorithm (1 = First Fit, 2 = Best Fit) (current %d): ", current == FIRST_FIT ? 1 : 2);
        fflush(stdout);

        if(!readLine(line, sizeof line)){ return current; }
        if(isBlank(line)){ return current; }

        long value;
        if(parseInt(line, &value) && value == 1){ return FIRST_FIT; }
        if(parseInt(line, &value) && value == 2){ return BEST_FIT; }
        printf("  Invalid value: enter 1 or 2.\n");
    }
}

bool terminalConfigure(Settings *settings){
    printf("\n CONFIGURATION \n");
    printf("(Enter keeps the current value)\n");

    int mem = askMemorySize(settings->memory_size);

    int max_kb = mem * 1024; // a process can't be bigger than the memory
    int current_p = settings->max_memory_process > max_kb ? max_kb : settings->max_memory_process;
        int max_p = askInt("Maximum memory size per process (KB)", current_p, 1, max_kb);;

    int sys_q = askInt("System quantum (Round Robin)", settings->system_quantum, 1, INT_MAX);
    int max_q = askInt("Maximum processing quantum per process", settings->max_quantum_process, 1, INT_MAX);
    FitAlgorithm algorithm = askAlgorithm(settings->algorithm);

    if(!changeSettings(settings, sys_q, max_p, max_q, mem)){ return false; }
    settings->algorithm = algorithm;
    return true;
}