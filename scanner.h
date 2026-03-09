#ifndef SCANNER_H
#define SCANNER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_HOSTS 64
#define MAX_PORTS 1024
#define TIMEOUT_MS 1000

typedef enum {
    PORT_CLOSED,
    PORT_OPEN,
    PORT_FILTERED
} PortStatus;

typedef struct {
    int port;
    PortStatus status;
} PortResult;

typedef struct {
    char hostname[256];
    PortResult ports[MAX_PORTS];
    int port_count;
    int open_count;
    int closed_count;
    int filtered_count;
} HostResult;

typedef struct {
    HostResult hosts[MAX_HOSTS];
    int host_count;
    int total_ports;
    int scanned_ports;
    int open_ports;
    bool running;
    pthread_mutex_t mutex;
} ScanContext;

void scanner_init(ScanContext *ctx);
void scanner_cleanup(ScanContext *ctx);

int scanner_parse_ports(const char *input, int *ports, int max_ports);
int scanner_parse_hosts(const char *input, char hosts[MAX_HOSTS][256], int max_hosts);

void scanner_scan_host(ScanContext *ctx, const char *hostname, const int *ports, int port_count);
void scanner_scan_all(ScanContext *ctx, const char hosts[MAX_HOSTS][256], int host_count, const int *ports, int port_count);

void scanner_stop(ScanContext *ctx);

const char* scanner_status_str(PortStatus status);

#endif
