#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

void history_init(History *hist) {
    memset(hist, 0, sizeof(History));
    hist->current_index = -1;
}

void history_cleanup(History *hist) {
    (void)hist;
}

void history_add(History *hist, const char hosts[MAX_HOSTS][256], int host_count, const HostResult *results, int total_scanned, int open_ports) {
    if (hist->count >= MAX_HISTORY_ENTRIES) {
        for (int i = 0; i < MAX_HISTORY_ENTRIES - 1; i++) {
            hist->entries[i] = hist->entries[i + 1];
        }
        hist->count = MAX_HISTORY_ENTRIES - 1;
    }

    HistoryEntry *entry = &hist->entries[hist->count];
    
    time_t now = time(NULL);
    entry->timestamp = now;
    snprintf(entry->id, sizeof(entry->id), "%ld", (long)now);
    
    entry->host_count = host_count;
    for (int i = 0; i < host_count && i < MAX_HOSTS; i++) {
        strncpy(entry->hosts[i], hosts[i], 255);
    }
    
    if (results) {
        entry->results = *results;
    }
    
    entry->total_scanned = total_scanned;
    entry->open_ports = open_ports;
    
    hist->count++;
    hist->current_index = hist->count - 1;
}

HistoryEntry* history_get(History *hist, int index) {
    if (index < 0 || index >= hist->count) {
        return NULL;
    }
    return &hist->entries[index];
}

HistoryEntry* history_get_latest(History *hist) {
    if (hist->count == 0) {
        return NULL;
    }
    return &hist->entries[hist->count - 1];
}

int history_save_json(const History *hist, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return -1;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"count\": %d,\n", hist->count);
    fprintf(fp, "  \"entries\": [\n");

    for (int i = 0; i < hist->count; i++) {
        const HistoryEntry *e = &hist->entries[i];
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"id\": \"%s\",\n", e->id);
        fprintf(fp, "      \"timestamp\": %ld,\n", (long)e->timestamp);
        fprintf(fp, "      \"hosts\": [");
        for (int j = 0; j < e->host_count; j++) {
            fprintf(fp, "\"%s\"%s", e->hosts[j], j < e->host_count - 1 ? ", " : "");
        }
        fprintf(fp, "],\n");
        fprintf(fp, "      \"total_scanned\": %d,\n", e->total_scanned);
        fprintf(fp, "      \"open_ports\": %d,\n", e->open_ports);
        fprintf(fp, "      \"results\": {\n");
        fprintf(fp, "        \"hostname\": \"%s\",\n", e->results.hostname);
        fprintf(fp, "        \"port_count\": %d,\n", e->results.port_count);
        fprintf(fp, "        \"open_count\": %d,\n", e->results.open_count);
        fprintf(fp, "        \"closed_count\": %d,\n", e->results.closed_count);
        fprintf(fp, "        \"filtered_count\": %d\n", e->results.filtered_count);
        fprintf(fp, "      }\n");
        fprintf(fp, "    }%s\n", i < hist->count - 1 ? "," : "");
    }

    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
    return 0;
}

int history_load_json(History *hist, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *content = malloc(fsize + 1);
    fread(content, 1, fsize, fp);
    fclose(fp);
    content[fsize] = 0;

    free(content);
    return 0;
}

int history_export_results(const HostResult *results, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        return -1;
    }

    fprintf(fp, "{\n");
    fprintf(fp, "  \"hostname\": \"%s\",\n", results->hostname);
    fprintf(fp, "  \"scanned_at\": %ld,\n", (long)time(NULL));
    fprintf(fp, "  \"summary\": {\n");
    fprintf(fp, "    \"total_ports\": %d,\n", results->port_count);
    fprintf(fp, "    \"open\": %d,\n", results->open_count);
    fprintf(fp, "    \"closed\": %d,\n", results->closed_count);
    fprintf(fp, "    \"filtered\": %d\n", results->filtered_count);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"ports\": [\n");

    for (int i = 0; i < results->port_count; i++) {
        fprintf(fp, "    {\"port\": %d, \"status\": \"%s\"}%s\n",
                results->ports[i].port,
                scanner_status_str(results->ports[i].status),
                i < results->port_count - 1 ? "," : "");
    }

    fprintf(fp, "  ]\n");
    fprintf(fp, "}\n");

    fclose(fp);
    return 0;
}

void history_list(const History *hist) {
    printf("\n=== Scan History ===\n");
    if (hist->count == 0) {
        printf("No scan history.\n");
        return;
    }

    for (int i = 0; i < hist->count; i++) {
        const HistoryEntry *e = &hist->entries[i];
        struct tm *tm_info = localtime(&e->timestamp);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

        printf("[%d] %s - %s\n", i + 1, time_str, e->hosts[0]);
        printf("    Scanned: %d ports | Open: %d\n", e->total_scanned, e->open_ports);
    }
}
