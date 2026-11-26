#include "../headers/stadistics.h"

#include <stdio.h>
#include <string.h>

void statsInit(Statistics *stats, size_t total_memory_kb) {
    if (!stats) return;

    memset(stats, 0, sizeof(*stats));
    stats->total_memory_kb = total_memory_kb;
    stats->current_free_kb = total_memory_kb;
}

void statsRecordCycle(Statistics *stats, const Memory *memory, long quantum_used_in_cycle) {
    if (!stats || !memory) return;

    size_t used_kb = 0;
    size_t free_kb = 0;
    size_t largest_hole_kb = 0;
    int holes = 0;

    for (int i = 0; i < memory->block_count; i++) {
        size_t block_size = (size_t)memory->blocks[i].size;
        if (memory->blocks[i].process != NULL) {
            used_kb += block_size;
        } else {
            free_kb += block_size;
            holes++;
            if (block_size > largest_hole_kb) {
                largest_hole_kb = block_size;
            }
        }
    }

    int cycles_elapsed = (quantum_used_in_cycle > 0) ? (int)quantum_used_in_cycle : 1;
    int prev_cycles = stats->total_cycles;
    stats->total_cycles += cycles_elapsed;

    // stats->total_cycles++;
    stats->current_used_kb = used_kb;
    stats->current_free_kb = free_kb;
    stats->current_holes = holes;

    if (used_kb > stats->peak_used_kb) {
        stats->peak_used_kb = used_kb;
    }

    if (stats->total_memory_kb > 0) {
        stats->current_memory_usage_pct =
            ((double)used_kb / (double)stats->total_memory_kb) * 100.0;
        // stats->avg_memory_usage_pct =
        //     ((stats->avg_memory_usage_pct * (double)(stats->total_cycles - 1)) +
        //      stats->current_memory_usage_pct) / (double)stats->total_cycles;
        stats->avg_memory_usage_pct =
            ((stats->avg_memory_usage_pct * (double)(prev_cycles)) +
             (stats->current_memory_usage_pct * (double)(cycles_elapsed))) / (double)stats->total_cycles;
    }

    double ext_frag = 0.0;
    if (free_kb > 0) {
        ext_frag = ((double)(free_kb - largest_hole_kb) / (double)free_kb) * 100.0;
    }
    // stats->avg_external_fragmentation_pct =
    //     ((stats->avg_external_fragmentation_pct * (double)(stats->total_cycles - 1)) +
    //      ext_frag) / (double)stats->total_cycles;
    stats->avg_external_fragmentation_pct =
        ((stats->avg_external_fragmentation_pct * (double)prev_cycles) +
         (ext_frag * (double)cycles_elapsed)) / (double)stats->total_cycles;


    if (quantum_used_in_cycle > 0) {
        stats->total_quantum_served += quantum_used_in_cycle;
    }
}

void statsRecordProcessGenerated(Statistics *stats) {
    if (!stats) return;
    stats->processes_generated++;
}

void statsRecordProcessRejected(Statistics *stats) {
    if (!stats) return;
    stats->processes_rejected++;
}

void statsRecordProcessCompleted(Statistics *stats, long quantum_consumed, long waiting_time) {
    if (!stats) return;

    int n = stats->processes_completed;
    stats->avg_service_quantum =
        ((stats->avg_service_quantum * (double)n) + (double)quantum_consumed) / (double)(n + 1);
    stats->avg_waiting_time =
        ((stats->avg_waiting_time * (double)n) + (double)waiting_time) / (double)(n + 1);

    stats->processes_completed++;
}

void statsPrintLive(const Statistics *stats) {
    if (!stats) return;

    printf("[LIVE | Cycle %d] Memory: %zu/%zu KB (%.1f%%) | Holes: %d | Attended to: %d\n",
           stats->total_cycles,
           stats->current_used_kb,
           stats->total_memory_kb,
           stats->current_memory_usage_pct,
           stats->current_holes,
           stats->processes_completed);
}

void statsPrintSummary(const Statistics *stats) {
    if (!stats) return;

    printf("\n==== SIMULATION STATISTICS ====\n");
    printf("Processes handled               : %d processes\n", stats->processes_completed);
    printf("Average wait time               : %.2f cycles\n", stats->avg_waiting_time);
    printf("Average memory usage            : %.2f%%\n", stats->avg_memory_usage_pct);
    printf("---- Additional indicators ----\n");
    printf("Executed cycles                 : %d cycles\n", stats->total_cycles);
    printf("Generated processes             : %d processes\n", stats->processes_generated);
    printf("Allocation failures             : %d attemps\n", stats->processes_rejected);
    printf("CPU active time                 : %ld cycles\n", stats->total_quantum_served);
    printf("Average external fragmentation  : %.2f%%\n", stats->avg_external_fragmentation_pct);
    printf("Peak memory usage               : %zu / %zu KB\n",
           stats->peak_used_kb, stats->total_memory_kb);
    printf("==================================\n");
}
