#ifndef TUI_H
#define TUI_H

#include <stdbool.h>
#include "scanner.h"
#include "history.h"

typedef enum {
    TUI_MODE_HOSTS,
    TUI_MODE_PORTS,
    TUI_MODE_SCANNING,
    TUI_MODE_RESULTS,
    TUI_MODE_HISTORY,
    TUI_MODE_HELP
} TUIMode;

typedef struct {
    TUIMode mode;
    char hosts_input[1024];
    char ports_input[256];
    ScanContext scan_ctx;
    History history;
    int selected_host;
    int selected_result;
    bool running;
} TUIContext;

void tui_init(TUIContext *ctx);
void tui_cleanup(TUIContext *ctx);
void tui_run(TUIContext *ctx);

void tui_draw_header(const TUIContext *ctx);
void tui_draw_hosts_input(TUIContext *ctx);
void tui_draw_ports_input(TUIContext *ctx);
void tui_draw_results(const TUIContext *ctx);
void tui_draw_history(const TUIContext *ctx);
void tui_draw_help(const TUIContext *ctx);
void tui_draw_status_bar(const TUIContext *ctx);

void tui_handle_input(TUIContext *ctx, int ch);

void tui_start_scan(TUIContext *ctx);
void tui_stop_scan(TUIContext *ctx);

void tui_set_status(TUIContext *ctx, const char *status);

#endif
