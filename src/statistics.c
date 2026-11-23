#include "../headers/statistics.h"

#include <stdio.h>
#include <string.h>

void statsInit(Statistics *stats, size_t total_memory_kb){
    if(!stats)
        return;

    memset(stats, 0, sizeof(*stats));
    stats->total_memory_kb = total_memory_kb;
    stats->current_free_kb = total_memory_kb;
}

void statsRecordCycle(Statistics *stats, size_t used_kb, long quantum_used_in_cycle){
    if(!stats)
        return;

    if(used_kb > stats->total_memory_kb)
        used_kb = stats->total_memory_kb;

    stats->total_cycles++;
    stats->current_used_kb = used_kb;
    stats->current_free_kb = stats->total_memory_kb - used_kb;

    if(used_kb > stats->peak_used_kb)
        stats->peak_used_kb = used_kb;

    if(stats->total_memory_kb > 0){
        stats->current_memory_usage_pct = ((double)used_kb / (double)stats->total_memory_kb) * 100.0;
        stats->avg_memory_usage_pct = ((stats->avg_memory_usage_pct * (double)(stats->total_cycles - 1)) 
                                      + stats->current_memory_usage_pct) / (double)stats->total_cycles;
    }
    
    if(quantum_used_in_cycle > 0)
        stats->total_quantum_served += quantum_used_in_cycle;
}

void statsRecordProcessGenerated(Statistics *stats){
    if(!stats)
        return;
    
    stats->processes_generated++;
}

void statsRecordProcessRejected(Statistics *stats) {
    if (!stats) return;
    stats->processes_rejected++;
}

void statsRecordProcessCompleted(Statistics *stats, long quantum_consumed, long turnaround_cycles) {
    if (!stats) return;

    int n = stats->processes_completed;
    stats->avg_service_quantum =
        ((stats->avg_service_quantum * (double)n) + (double)quantum_consumed) / (double)(n + 1);

    if (turnaround_cycles >= 0) {
        stats->avg_turnaround_cycles =
            ((stats->avg_turnaround_cycles * (double)n) + (double)turnaround_cycles) / (double)(n + 1);
    }

    stats->processes_completed++;
}

void statsPrintLive(const Statistics *stats) {
    if (!stats) return;

    printf("[LIVE | Ciclo %d] Mem: %zu/%zu KB (%.1f%%) | Atendidos: %d\n",
           stats->total_cycles,
           stats->current_used_kb,
           stats->total_memory_kb,
           stats->current_memory_usage_pct,
           stats->processes_completed);
}

void statsPrintSummary(const Statistics *stats) {
    if (!stats) return;

    printf("\n=== ESTADISTICAS DE SIMULACION ===\n");
    printf("Procesos atendidos          : %d\n", stats->processes_completed);
    printf("Cuanto promedio de atencion : %.2f\n", stats->avg_service_quantum);
    printf("Ocupacion promedio memoria  : %.2f%%\n", stats->avg_memory_usage_pct);
    printf("--- Indicadores adicionales ---\n");
    printf("Ciclos ejecutados            : %d\n", stats->total_cycles);
    printf("Procesos generados          : %d\n", stats->processes_generated);
    printf("Procesos rechazados         : %d\n", stats->processes_rejected);
    printf("Cuanto total servido          : %ld\n", stats->total_quantum_served);
    printf("Ciclos retorno promedio     : %.2f\n", stats->avg_turnaround_cycles);
    printf("Pico de memoria usada         : %zu / %zu KB\n",
           stats->peak_used_kb, stats->total_memory_kb);
    printf("==================================\n");
}