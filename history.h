#ifndef HISTORY_H
#define HISTORY_H

#include <time.h>
#include "scanner.h"

#define MAX_HISTORY_ENTRIES 100

typedef struct {
    char id[64];
    time_t timestamp;
    char hosts[MAX_HOSTS][256];
    int host_count;
    int total_scanned;
    int open_ports;
    HostResult results;
} HistoryEntry;

typedef struct {
    HistoryEntry entries[MAX_HISTORY_ENTRIES];
    int count;
    int current_index;
} History;

void history_init(History *hist);
void history_cleanup(History *hist);

void history_add(History *hist, const char hosts[MAX_HOSTS][256], int host_count, const HostResult *results, int total_scanned, int open_ports);

HistoryEntry* history_get(History *hist, int index);
HistoryEntry* history_get_latest(History *hist);

int history_save_json(const History *hist, const char *filename);
int history_load_json(History *hist, const char *filename);

int history_export_results(const HostResult *results, const char *filename);

void history_list(const History *hist);

#endif
