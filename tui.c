#include "tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <menu.h>
#include <panel.h>
#include <form.h>
#include <time.h>
#include <term.h>

#ifndef KEY_F1
#define KEY_F1 265
#endif

#define MAX_HOSTS 64
#define MAX_PORTS 1024

static const char *status_message = "Welcome to PortCheck TUI | Press F1 for help";

static void tui_draw_box(int y1, int x1, int y2, int x2, bool shadow) {
    if (shadow) {
        attron(COLOR_PAIR(8));
        for (int y = y1 + 1; y <= y2 + 1; y++) {
            for (int x = x1 + 1; x <= x2 + 1; x++) {
                mvaddch(y, x, ' ');
            }
        }
        attroff(COLOR_PAIR(8));
    }
    
    attron(COLOR_PAIR(1));
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
    
    for (int x = x1 + 1; x < x2; x++) {
        mvaddch(y1, x, ACS_HLINE);
        mvaddch(y2, x, ACS_HLINE);
    }
    for (int y = y1 + 1; y < y2; y++) {
        mvaddch(y, x1, ACS_VLINE);
        mvaddch(y, x2, ACS_VLINE);
    }
    attroff(COLOR_PAIR(1));
}

static void tui_draw_progress_bar(int y, int x, int width, float progress) {
    int filled = (int)(progress * width / 100.0);
    
    attron(COLOR_PAIR(1));
    mvaddch(y, x, '[');
    mvaddch(y, x + width + 1, ']');
    attroff(COLOR_PAIR(1));
    
    for (int i = 0; i < filled; i++) {
        attron(COLOR_PAIR(2) | A_REVERSE);
        mvaddch(y, x + 1 + i, ' ');
        attroff(COLOR_PAIR(2) | A_REVERSE);
    }
    
    for (int i = filled; i < width; i++) {
        mvaddch(y, x + 1 + i, ' ');
    }
    
    char pct[16];
    snprintf(pct, sizeof(pct), "%.1f%%", progress);
    mvprintw(y, x + width / 2 - 2, "%s", pct);
}

void tui_init(TUIContext *ctx) {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_WHITE, COLOR_BLACK);
    init_pair(7, COLOR_BLACK, COLOR_WHITE);
    init_pair(8, COLOR_BLACK, COLOR_BLACK);
    
    memset(ctx, 0, sizeof(TUIContext));
    ctx->mode = TUI_MODE_HOSTS;
    strcpy(ctx->hosts_input, "");
    strcpy(ctx->ports_input, "22,80,443,8080");
    ctx->running = true;
    
    scanner_init(&ctx->scan_ctx);
    history_init(&ctx->history);
}

void tui_cleanup(TUIContext *ctx) {
    scanner_cleanup(&ctx->scan_ctx);
    history_cleanup(&ctx->history);
    endwin();
}

static void draw_title_bar(void) {
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(0, 0, " PortCheck TUI v2.0 ");
    attroff(COLOR_PAIR(1) | A_BOLD);
    
    for (int x = 19; x < COLS; x++) {
        mvaddch(0, x, ' ');
    }
}

static void draw_mode_indicator(TUIMode mode) {
    const char *mode_name;
    switch (mode) {
        case TUI_MODE_HOSTS: mode_name = "HOSTS"; break;
        case TUI_MODE_PORTS: mode_name = "PORTS"; break;
        case TUI_MODE_SCANNING: mode_name = "SCANNING"; break;
        case TUI_MODE_RESULTS: mode_name = "RESULTS"; break;
        case TUI_MODE_HISTORY: mode_name = "HISTORY"; break;
        case TUI_MODE_HELP: mode_name = "HELP"; break;
        default: mode_name = "UNKNOWN";
    }
    
    attron(COLOR_PAIR(7));
    mvprintw(0, COLS - 15, "[ %s ]", mode_name);
    attroff(COLOR_PAIR(7));
}

void tui_draw_header(const TUIContext *ctx) {
    draw_title_bar();
    draw_mode_indicator(ctx->mode);
    
    attron(COLOR_PAIR(6));
    mvprintw(2, 2, "┌─────────────────────────────────────────────────────────────────────────────┐");
    mvprintw(3, 2, "│  PortCheck - TCP Port Scanner                                            │");
    mvprintw(4, 2, "│  Multi-host | Port Range | History | Export                             │");
    mvprintw(5, 2, "└─────────────────────────────────────────────────────────────────────────────┘");
    attroff(COLOR_PAIR(6));
}

void tui_draw_hosts_input(TUIContext *ctx) {
    int y = 8;
    
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y, 2, " Hosts Input ");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    tui_draw_box(y + 1, 2, y + 6, 50, true);
    
    attron(COLOR_PAIR(6));
    mvprintw(y + 2, 4, "Enter hosts (comma separated or one per line):");
    mvprintw(y + 3, 4, "Examples: google.com, localhost, 192.168.1.1");
    attroff(COLOR_PAIR(6));
    
    if (ctx->mode == TUI_MODE_HOSTS) {
        attron(COLOR_PAIR(2) | A_REVERSE);
    }
    mvprintw(y + 4, 4, "> %-42s", ctx->hosts_input);
    if (ctx->mode == TUI_MODE_HOSTS) {
        attroff(COLOR_PAIR(2) | A_REVERSE);
    }
    
    mvprintw(y + 8, 2, " Press [TAB] to switch to Ports  |  Press [ENTER] to start scan");
}

void tui_draw_ports_input(TUIContext *ctx) {
    int y = 8;
    int x = 55;
    
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y, x, " Ports Input ");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    tui_draw_box(y + 1, x - 2, y + 6, x + 42, true);
    
    attron(COLOR_PAIR(6));
    mvprintw(y + 2, x, "Enter ports/range:");
    mvprintw(y + 3, x, "Examples: 80,443,8080 or 1-1000");
    attroff(COLOR_PAIR(6));
    
    if (ctx->mode == TUI_MODE_PORTS) {
        attron(COLOR_PAIR(2) | A_REVERSE);
    }
    mvprintw(y + 4, x, "> %-36s", ctx->ports_input);
    if (ctx->mode == TUI_MODE_PORTS) {
        attroff(COLOR_PAIR(2) | A_REVERSE);
    }
    
    mvprintw(y + 8, x - 2, " Press [TAB] to switch to Hosts  |  Press [ENTER] to start scan");
}

void tui_draw_results(const TUIContext *ctx) {
    int y = 16;
    
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y - 1, 2, " Scan Results ");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    tui_draw_box(y, 2, y + 18, COLS - 3, false);
    
    if (ctx->scan_ctx.host_count == 0) {
        attron(COLOR_PAIR(6));
        mvprintw(y + 8, COLS / 2 - 15, "No scan results yet. Run a scan!");
        attroff(COLOR_PAIR(6));
        return;
    }
    
    mvprintw(y + 1, 4, "Host");
    mvprintw(y + 1, 40, "Port");
    mvprintw(y + 1, 55, "Status");
    mvprintw(y + 1, 70, "Host");
    mvprintw(y + 1, 106, "Port");
    mvprintw(y + 1, 121, "Status");
    
    attron(COLOR_PAIR(6));
    for (int i = 0; i < COLS - 7; i++) {
        mvaddch(y + 2, 4 + i, '-');
    }
    attroff(COLOR_PAIR(6));
    
    int row = y + 3;
    for (int i = 0; i < ctx->scan_ctx.host_count && row < y + 16; i++) {
        const HostResult *hr = &ctx->scan_ctx.hosts[i];
        
        for (int j = 0; j < 8 && (i * 8 + j) < hr->port_count; j++) {
            const PortResult *pr = &hr->ports[i * 8 + j];
            
            int col = (j < 4) ? 4 : 70;
            int r = row + (j < 4 ? j : j - 4);
            
            mvprintw(r, col, "%-34s", hr->hostname);
            mvprintw(r, col + 36, "%-12d", pr->port);
            
            switch (pr->status) {
                case PORT_OPEN:
                    attron(COLOR_PAIR(2) | A_BOLD);
                    mvprintw(r, col + 50, "OPEN");
                    attroff(COLOR_PAIR(2) | A_BOLD);
                    break;
                case PORT_CLOSED:
                    attron(COLOR_PAIR(3));
                    mvprintw(r, col + 50, "CLOSED");
                    attroff(COLOR_PAIR(3));
                    break;
                case PORT_FILTERED:
                    attron(COLOR_PAIR(4));
                    mvprintw(r, col + 50, "FILTERED");
                    attroff(COLOR_PAIR(4));
                    break;
            }
        }
        row += 4;
    }
    
    attron(COLOR_PAIR(6));
    mvprintw(y + 17, 4, "Total: %d hosts | Open: %d | Closed: %d | Filtered: %d",
            ctx->scan_ctx.host_count,
            ctx->scan_ctx.open_ports,
            ctx->scan_ctx.hosts[0].closed_count,
            ctx->scan_ctx.hosts[0].filtered_count);
    attroff(COLOR_PAIR(6));
}

void tui_draw_history(const TUIContext *ctx) {
    int y = 16;
    
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y - 1, 2, " Scan History ");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    tui_draw_box(y, 2, y + 18, COLS - 3, false);
    
    if (ctx->history.count == 0) {
        attron(COLOR_PAIR(6));
        mvprintw(y + 8, COLS / 2 - 12, "No scan history yet.");
        attroff(COLOR_PAIR(6));
        return;
    }
    
    mvprintw(y + 1, 4, "#");
    mvprintw(y + 1, 10, "Timestamp");
    mvprintw(y + 1, 35, "Host");
    mvprintw(y + 1, 70, "Ports Scanned");
    mvprintw(y + 1, 90, "Open");
    
    attron(COLOR_PAIR(6));
    for (int i = 0; i < COLS - 7; i++) {
        mvaddch(y + 2, 4 + i, '-');
    }
    attroff(COLOR_PAIR(6));
    
    for (int i = 0; i < ctx->history.count && i < 14; i++) {
        const HistoryEntry *e = &ctx->history.entries[i];
        struct tm *tm_info = localtime(&e->timestamp);
        char time_str[32];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);
        
        if (i == ctx->history.current_index) {
            attron(COLOR_PAIR(2) | A_REVERSE);
        }
        
        mvprintw(y + 3 + i, 4, "%-4d", i + 1);
        mvprintw(y + 3 + i, 10, "%s", time_str);
        mvprintw(y + 3 + i, 35, "%-33s", e->hosts[0]);
        mvprintw(y + 3 + i, 70, "%d", e->total_scanned);
        
        attron(COLOR_PAIR(2) | A_BOLD);
        mvprintw(y + 3 + i, 90, "%d", e->open_ports);
        attroff(COLOR_PAIR(2) | A_BOLD);
        
        if (i == ctx->history.current_index) {
            attroff(COLOR_PAIR(2) | A_REVERSE);
        }
    }
    
    mvprintw(y + 17, 4, "[S] Save to JSON | [L] Load | [E] Export Results | [C] Clear History");
}

void tui_draw_help(const TUIContext *ctx) {
    (void)ctx;
    int y = 8;
    int x = COLS / 2 - 30;
    
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(y - 1, x + 20, " Help / Keyboard Shortcuts ");
    attroff(COLOR_PAIR(1) | A_BOLD);
    
    tui_draw_box(y, x, y + 20, x + 60, true);
    
    attron(COLOR_PAIR(6));
    const char *help[] = {
        " [TAB]     - Switch between Hosts/Ports fields",
        " [ENTER]   - Start scan",
        " [ESC]     - Cancel / Back",
        " [R]       - Show Results",
        " [H]       - Show History",
        " [S]       - Save results to JSON",
        " [E]       - Export current results",
        " [1-4]     - Quick port presets",
        "           1: Common (22,80,443,...)",
        "           2: Web (80,443,8080,8443)",
        "           3: Database (3306,5432,27017)",
        "           4: Custom range",
        " [F1/?]    - Toggle help",
        " [Q]       - Quit",
        "",
        " Status colors:",
        "   GREEN   - Port OPEN",
        "   RED     - Port CLOSED",
        "   YELLOW  - Port FILTERED"
    };
    
    for (int i = 0; help[i][0] != '\0' || i == 14; i++) {
        if (help[i][0] == '\0') {
            attron(COLOR_PAIR(4));
            mvprintw(y + 1 + i, x + 2, "%s", help[i]);
            attroff(COLOR_PAIR(4));
        } else {
            mvprintw(y + 1 + i, x + 2, "%s", help[i]);
        }
    }
    attroff(COLOR_PAIR(6));
}

void tui_draw_status_bar(const TUIContext *ctx) {
    attron(COLOR_PAIR(7));
    for (int x = 0; x < COLS; x++) {
        mvaddch(LINES - 1, x, ' ');
    }
    mvprintw(LINES - 1, 2, "%s", status_message);
    attroff(COLOR_PAIR(7));
}

void tui_set_status(TUIContext *ctx, const char *status) {
    (void)ctx;
    status_message = status;
}

void tui_start_scan(TUIContext *ctx) {
    if (strlen(ctx->hosts_input) == 0) {
        tui_set_status(ctx, "Error: Please enter at least one host");
        return;
    }
    
    if (strlen(ctx->ports_input) == 0) {
        tui_set_status(ctx, "Error: Please enter at least one port");
        return;
    }
    
    char hosts[MAX_HOSTS][256];
    int ports[MAX_PORTS];
    
    int host_count = scanner_parse_hosts(ctx->hosts_input, hosts, MAX_HOSTS);
    int port_count = scanner_parse_ports(ctx->ports_input, ports, MAX_PORTS);
    
    if (host_count == 0) {
        tui_set_status(ctx, "Error: No valid hosts found");
        return;
    }
    
    if (port_count == 0) {
        tui_set_status(ctx, "Error: No valid ports found");
        return;
    }
    
    ctx->mode = TUI_MODE_SCANNING;
    tui_set_status(ctx, "Scanning in progress...");
    
    scanner_scan_all(&ctx->scan_ctx, hosts, host_count, ports, port_count);
    
    history_add(&ctx->history, hosts, host_count, &ctx->scan_ctx.hosts[0], 
                ctx->scan_ctx.scanned_ports, ctx->scan_ctx.open_ports);
    
    ctx->mode = TUI_MODE_RESULTS;
    tui_set_status(ctx, "Scan complete! Press [R] to view results");
}

void tui_stop_scan(TUIContext *ctx) {
    scanner_stop(&ctx->scan_ctx);
    ctx->mode = TUI_MODE_HOSTS;
    tui_set_status(ctx, "Scan cancelled");
}

void tui_handle_input(TUIContext *ctx, int ch) {
    switch (ch) {
        case 27:
            if (ctx->mode == TUI_MODE_HELP) {
                ctx->mode = TUI_MODE_HOSTS;
            } else if (ctx->mode == TUI_MODE_SCANNING) {
                tui_stop_scan(ctx);
            } else if (ctx->mode == TUI_MODE_RESULTS || ctx->mode == TUI_MODE_HISTORY) {
                ctx->mode = TUI_MODE_HOSTS;
            }
            break;
            
        case '\t':
            if (ctx->mode == TUI_MODE_HOSTS) {
                ctx->mode = TUI_MODE_PORTS;
            } else if (ctx->mode == TUI_MODE_PORTS) {
                ctx->mode = TUI_MODE_HOSTS;
            }
            break;
            
        case '\n':
            if (ctx->mode == TUI_MODE_HOSTS || ctx->mode == TUI_MODE_PORTS) {
                tui_start_scan(ctx);
            }
            break;
            
        case KEY_F1:
        case '?':
            if (ctx->mode == TUI_MODE_HELP) {
                ctx->mode = TUI_MODE_HOSTS;
            } else {
                ctx->mode = TUI_MODE_HELP;
            }
            break;
            
        case 'q':
        case 'Q':
            if (ctx->mode == TUI_MODE_HELP) {
                ctx->mode = TUI_MODE_HOSTS;
            } else {
                ctx->running = false;
            }
            break;
            
        case 'r':
        case 'R':
            ctx->mode = TUI_MODE_RESULTS;
            break;
            
        case 'h':
        case 'H':
            ctx->mode = TUI_MODE_HISTORY;
            break;
            
        case 's':
        case 'S':
            if (ctx->history.count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "portcheck_%ld.json", (long)time(NULL));
                history_save_json(&ctx->history, filename);
                tui_set_status(ctx, "History saved to JSON");
            }
            break;
            
        case 'e':
        case 'E':
            if (ctx->scan_ctx.host_count > 0) {
                char filename[128];
                snprintf(filename, sizeof(filename), "scan_%s_%ld.json", 
                        ctx->scan_ctx.hosts[0].hostname, (long)time(NULL));
                history_export_results(&ctx->scan_ctx.hosts[0], filename);
                tui_set_status(ctx, "Results exported to JSON");
            }
            break;
            
        case '1':
            if (ctx->mode == TUI_MODE_HOSTS || ctx->mode == TUI_MODE_PORTS) {
                strcpy(ctx->ports_input, "21,22,23,25,53,80,110,143,443,993,995,3306,3389,5432,8080,8443");
                tui_set_status(ctx, "Port preset: Common ports selected");
            }
            break;
            
        case '2':
            if (ctx->mode == TUI_MODE_HOSTS || ctx->mode == TUI_MODE_PORTS) {
                strcpy(ctx->ports_input, "80,443,8080,8443,8000,8888,3000");
                tui_set_status(ctx, "Port preset: Web ports selected");
            }
            break;
            
        case '3':
            if (ctx->mode == TUI_MODE_HOSTS || ctx->mode == TUI_MODE_PORTS) {
                strcpy(ctx->ports_input, "3306,5432,27017,6379,9200,9300,27018");
                tui_set_status(ctx, "Port preset: Database ports selected");
            }
            break;
            
        case '4':
            if (ctx->mode == TUI_MODE_HOSTS || ctx->mode == TUI_MODE_PORTS) {
                strcpy(ctx->ports_input, "1-1000");
                tui_set_status(ctx, "Port preset: Range 1-1000 selected");
            }
            break;
            
        case KEY_BACKSPACE:
        case 127:
            if (ctx->mode == TUI_MODE_HOSTS) {
                size_t len = strlen(ctx->hosts_input);
                if (len > 0) ctx->hosts_input[len - 1] = '\0';
            } else if (ctx->mode == TUI_MODE_PORTS) {
                size_t len = strlen(ctx->ports_input);
                if (len > 0) ctx->ports_input[len - 1] = '\0';
            }
            break;
            
        default:
            if (ch >= 32 && ch <= 126) {
                if (ctx->mode == TUI_MODE_HOSTS) {
                    size_t len = strlen(ctx->hosts_input);
                    if (len < sizeof(ctx->hosts_input) - 1) {
                        ctx->hosts_input[len] = ch;
                        ctx->hosts_input[len + 1] = '\0';
                    }
                } else if (ctx->mode == TUI_MODE_PORTS) {
                    size_t len = strlen(ctx->ports_input);
                    if (len < sizeof(ctx->ports_input) - 1) {
                        ctx->ports_input[len] = ch;
                        ctx->ports_input[len + 1] = '\0';
                    }
                }
            }
            break;
    }
}

void tui_run(TUIContext *ctx) {
    while (ctx->running) {
        clear();
        
        tui_draw_header(ctx);
        
        if (ctx->mode != TUI_MODE_SCANNING && ctx->mode != TUI_MODE_RESULTS && ctx->mode != TUI_MODE_HISTORY) {
            tui_draw_hosts_input(ctx);
            tui_draw_ports_input(ctx);
        }
        
        if (ctx->mode == TUI_MODE_RESULTS) {
            tui_draw_results(ctx);
        }
        
        if (ctx->mode == TUI_MODE_HISTORY) {
            tui_draw_history(ctx);
        }
        
        if (ctx->mode == TUI_MODE_HELP) {
            tui_draw_help(ctx);
        }
        
        if (ctx->mode == TUI_MODE_SCANNING) {
            int y = 10;
            tui_draw_box(y, COLS / 2 - 20, y + 6, COLS / 2 + 20, true);
            
            attron(COLOR_PAIR(5) | A_BOLD);
            mvprintw(y + 1, COLS / 2 - 10, "Scanning in progress...");
            attroff(COLOR_PAIR(5) | A_BOLD);
            
            if (ctx->scan_ctx.total_ports > 0) {
                float progress = (float)ctx->scan_ctx.scanned_ports * 100.0 / ctx->scan_ctx.total_ports;
                tui_draw_progress_bar(y + 3, COLS / 2 - 15, 30, progress);
            }
            
            attron(COLOR_PAIR(6));
            mvprintw(y + 5, COLS / 2 - 12, "Press [ESC] to cancel");
            attroff(COLOR_PAIR(6));
        }
        
        tui_draw_status_bar(ctx);
        
        refresh();
        
        int ch = getch();
        tui_handle_input(ctx, ch);
        
        if (ctx->mode == TUI_MODE_SCANNING && !ctx->scan_ctx.running) {
            ctx->mode = TUI_MODE_RESULTS;
            tui_set_status(ctx, "Scan complete!");
        }
    }
}
