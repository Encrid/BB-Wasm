/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_HOST_H
    #define TEST_HOST_H

    #include <stddef.h>

    #include "bridge.h"
    #include "bridge_abi.h"

constexpr int64_t HOST_MS = 1'000'000;

enum host_setting : uint32_t {
    HOST_RESERVATION = 0,
    HOST_GUARD = 1,
    HOST_STACK = 2,
    HOST_MARGIN = 3,
    HOST_WATCHDOG = 4,
    HOST_LEVEL = 5,
    HOST_COMPILE = 6
};

constexpr uint32_t HOST_SETTINGS = 7;

typedef struct host {
    const char *what;
    const char *arg;
    void *kept;
    bridge_setting_t settings[HOST_SETTINGS];
    bridge_backend_t backends[1];
    const char *phases[2];
    bridge_config_t cfg;
    bridge_sink_fn_t sink;
    void *ud;
    bridge_world_t *w;
} host_t;

typedef struct {
    const char *name;
    void (*run)(host_t *h);
    const char *arg;
    void (*set)(host_t *h);
} host_case_t;

void host_init(host_t *h, int argc, char **argv);

int host_run(host_t *h, const host_case_t *cases, size_t count);

[[nodiscard]] int host_open(host_t *h);

void host_close(host_t *h);

void host_open_only(host_t *h);

void host_print_record(void *ud, const bridge_record_t *r);

void host_load(host_t *h, const bridge_artifact_t *a);
void host_reload(host_t *h, const bridge_artifact_t *a);
void host_unload(host_t *h, const char *identity);

void host_run_frames(host_t *h, int count);

void host_run_frames_unsaid(bridge_world_t *w, int count);

typedef struct {
    void (*run)(void *arg);
    void *arg;
    size_t stack;
    void *handle;
} host_thread_t;

void host_thread_start(host_thread_t *t);
void host_thread_join(host_thread_t *t);

void *host_read_module(const char *file, size_t *len);

void host_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file);

#endif
