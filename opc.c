#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <hostname_or_ip> <port>\n", argv[0]);
        return 1;
    }

    char *hostname = argv[1];
    int port = atoi(argv[2]);

    int sock;
    struct sockaddr_in server;
    struct hostent *host;

    host = gethostbyname(hostname);
    if (host == NULL) {
        fprintf(stderr, "Error: Unknown host %s\n", hostname);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(server.sin_zero), 0, 8);

    printf("Scanning host: %s port: %d\n", hostname, port);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == 0) {
        printf("Port %d is OPEN\n", port);
    } else {
        printf("Port %d is CLOSED\n", port);
    }

    close(sock);
    return 0;
}
