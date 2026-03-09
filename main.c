#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "tui.h"
#include "scanner.h"
#include "history.h"

static volatile int running = 1;

static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

static void print_banner(void) {
    printf("\n");
    printf("  ██████╗ ███████╗██╗   ██╗    ██████╗ ███████╗ ██████╗ █████╗ ██████╗ ███████╗\n");
    printf("  ██╔══██╗██╔════╝██║   ██║    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗██╔════╝\n");
    printf("  ██║  ██║█████╗  ██║   ██║    ██████╔╝█████╗  ██║     ███████║██████╔╝█████╗  \n");
    printf("  ██║  ██║██╔══╝  ╚██╗ ██╔╝    ██╔══██╗██╔══╝  ██║     ██╔══██║██╔══██╗██╔══╝  \n");
    printf("  ██████╔╝███████╗ ╚████╔╝     ██████╔╝███████╗╚██████╗██║  ██║██║  ██║███████╗\n");
    printf("  ╚═════╝ ╚══════╝  ╚═══╝      ╚═════╝ ╚══════╝ ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝\n");
    printf("\n");
    printf("  TCP Port Scanner - TUI Edition v2.0\n");
    printf("  ====================================\n");
    printf("\n");
}

static void print_usage(const char *prog) {
    printf("Usage: %s [OPTIONS]\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n");
    printf("  --cli          Run in CLI mode (legacy)\n");
    printf("\n");
    printf("Without options, runs in TUI mode.\n");
    printf("\n");
}

static void print_version(void) {
    printf("PortCheck TUI v2.0\n");
    printf("TCP Port Scanner with Terminal User Interface\n");
    printf("\n");
    printf("Features:\n");
    printf("  - Multi-host scanning\n");
    printf("  - Port range support\n");
    printf("  - Scan history\n");
    printf("  - JSON export\n");
    printf("  - Interactive TUI\n");
    printf("\n");
}

static int run_cli_mode(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        return 1;
    }
    
    char *hostname = argv[1];
    int port = atoi(argv[2]);
    
    printf("Scanning host: %s port: %d\n", hostname, port);
    
    ScanContext ctx;
    scanner_init(&ctx);
    
    int ports[1] = { port };
    scanner_scan_host(&ctx, hostname, ports, 1);
    
    const HostResult *result = &ctx.hosts[0];
    if (result->ports[0].status == PORT_OPEN) {
        printf("Port %d is OPEN\n", port);
    } else {
        printf("Port %d is CLOSED\n", port);
    }
    
    scanner_cleanup(&ctx);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            print_banner();
            print_usage(argv[0]);
            return 0;
        }
        
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
            print_version();
            return 0;
        }
        
        if (strcmp(argv[1], "--cli") == 0) {
            return run_cli_mode(argc - 1, argv + 1);
        }
        
        if (argc == 3) {
            return run_cli_mode(argc, argv);
        }
        
        print_usage(argv[0]);
        return 1;
    }
    
    print_banner();
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    TUIContext ctx;
    tui_init(&ctx);
    tui_run(&ctx);
    tui_cleanup(&ctx);
    
    printf("\nThank you for using PortCheck!\n");
    
    return 0;
}
