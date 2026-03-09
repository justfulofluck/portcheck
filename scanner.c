#define _POSIX_C_SOURCE 200809L

#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>

void scanner_init(ScanContext *ctx) {
    memset(ctx, 0, sizeof(ScanContext));
    pthread_mutex_init(&ctx->mutex, NULL);
    ctx->running = true;
}

void scanner_cleanup(ScanContext *ctx) {
    pthread_mutex_destroy(&ctx->mutex);
}

static int connect_port(const char *hostname, int port, int timeout_ms) {
    int sock;
    struct sockaddr_in server;
    struct hostent *host;
    struct pollfd pfd;
    int result;

    host = gethostbyname(hostname);
    if (host == NULL) {
        return -1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
    memset(&server.sin_zero, 0, 8);

    result = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if (result == 0) {
        close(sock);
        return 1;
    }

    if (errno != EINPROGRESS) {
        close(sock);
        return -1;
    }

    pfd.fd = sock;
    pfd.events = POLLOUT;
    result = poll(&pfd, 1, timeout_ms);

    if (result <= 0) {
        close(sock);
        return 0;
    }

    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

    close(sock);
    return (so_error == 0) ? 1 : 0;
}

int scanner_parse_ports(const char *input, int *ports, int max_ports) {
    int count = 0;
    char *input_copy = strdup(input);
    char *token = strtok(input_copy, ", \t\n");

    while (token && count < max_ports) {
        if (strchr(token, '-')) {
            int start, end;
            if (sscanf(token, "%d-%d", &start, &end) == 2) {
                for (int p = start; p <= end && count < max_ports; p++) {
                    if (p > 0 && p <= 65535) {
                        ports[count++] = p;
                    }
                }
            }
        } else {
            int port = atoi(token);
            if (port > 0 && port <= 65535) {
                ports[count++] = port;
            }
        }
        token = strtok(NULL, ", \t\n");
    }

    free(input_copy);
    return count;
}

int scanner_parse_hosts(const char *input, char hosts[MAX_HOSTS][256], int max_hosts) {
    int count = 0;
    char *input_copy = strdup(input);
    char *token = strtok(input_copy, ", \t\n");

    while (token && count < max_hosts) {
        strncpy(hosts[count], token, 255);
        hosts[count][255] = '\0';
        count++;
        token = strtok(NULL, ", \t\n");
    }

    free(input_copy);
    return count;
}

void scanner_scan_host(ScanContext *ctx, const char *hostname, const int *ports, int port_count) {
    HostResult *result = &ctx->hosts[ctx->host_count];
    strncpy(result->hostname, hostname, 255);
    result->hostname[255] = '\0';
    result->port_count = 0;
    result->open_count = 0;
    result->closed_count = 0;
    result->filtered_count = 0;

    for (int i = 0; i < port_count; i++) {
        PortStatus status;
        int conn_result = connect_port(hostname, ports[i], TIMEOUT_MS);

        if (conn_result > 0) {
            status = PORT_OPEN;
            result->open_count++;
            pthread_mutex_lock(&ctx->mutex);
            ctx->open_ports++;
            pthread_mutex_unlock(&ctx->mutex);
        } else if (conn_result == 0) {
            status = PORT_FILTERED;
            result->filtered_count++;
        } else {
            status = PORT_CLOSED;
            result->closed_count++;
        }

        result->ports[i].port = ports[i];
        result->ports[i].status = status;
        result->port_count++;

        pthread_mutex_lock(&ctx->mutex);
        ctx->scanned_ports++;
        pthread_mutex_unlock(&ctx->mutex);
    }

    ctx->host_count++;
}

void scanner_scan_all(ScanContext *ctx, const char hosts[MAX_HOSTS][256], int host_count, const int *ports, int port_count) {
    ctx->running = true;
    ctx->total_ports = host_count * port_count;
    ctx->scanned_ports = 0;
    ctx->open_ports = 0;
    ctx->host_count = 0;

    for (int i = 0; i < host_count && ctx->running; i++) {
        scanner_scan_host(ctx, hosts[i], ports, port_count);
    }
}

void scanner_stop(ScanContext *ctx) {
    ctx->running = false;
}

const char* scanner_status_str(PortStatus status) {
    switch (status) {
        case PORT_OPEN: return "OPEN";
        case PORT_CLOSED: return "CLOSED";
        case PORT_FILTERED: return "FILTERED";
        default: return "UNKNOWN";
    }
}
