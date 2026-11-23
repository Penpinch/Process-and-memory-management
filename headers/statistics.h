#ifndef STATISTICS_H
#define STATISTICS_H

#include "../headers/memory.h"
#include <stddef.h>

typedef struct {
    int total_cycles;
    int processes_generated;
    int processes_completed;
    int processes_rejected;

    long total_quantum_served;
    double avg_service_quantum;
    double avg_waiting_time;

    size_t total_memory_kb;
    size_t current_used_kb;
    size_t current_free_kb;
    size_t peak_used_kb;
    int current_holes;
    double avg_memory_usage_pct;
    double current_memory_usage_pct;
    double avg_external_fragmentation_pct;
} Statistics;

void statsInit(Statistics *stats, size_t total_memory_kb);
void statsRecordCycle(Statistics *stats, const Memory *memory, long quantum_used_in_cycle);
void statsRecordProcessGenerated(Statistics *stats);
void statsRecordProcessRejected(Statistics *stats);
void statsRecordProcessCompleted(Statistics *stats, long quantum_consumed, long waiting_time);
void statsPrintLive(const Statistics *stats);
void statsPrintSummary(const Statistics *stats);

#endif /* STATISTICS_H */