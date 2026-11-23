#ifndef STATISTICS_H
#define STATISTICS_H

#include <stddef.h>

typedef struct{
    int total_cycles;
    int processes_generated;
    int processes_completed;
    int processes_rejected;

    long total_quantum_served;
    double avg_service_quantum;
    double avg_turnaround_cycles;

    size_t total_memory_kb;
    size_t current_used_kb;
    size_t current_free_kb;
    size_t peak_used_kb;
    double avg_memory_usage_pct;
    double current_memory_usage_pct;
} Statistics;

void statsInit(Statistics *stats, size_t total_memory_kb);
void statsRecordCycle(Statistics *stats, size_t used_kb, long quantum_used_in_cycle);
void statsRecordProcessGenerated(Statistics *stats);
void statsRecordProcessRejected(Statistics *stats);
void statsRecordProcessCompleted(Statistics *stats, long quauntum_consumed, long turnaround_time);
void statsPrintLive(const Statistics *stats);
void statsPrintSummary(const Statistics *stats);

#endif