/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_ADOPT_H
    #define TEST_ADOPT_H

    #include "backend.h"
    #include "os.h"

constexpr uint32_t ADOPT_PAGE = 65'536;
constexpr size_t ADOPT_STACK = 1 << 20;
constexpr int64_t ADOPT_WASM_STACK = 512 << 10;
constexpr int64_t ADOPT_MARGIN = 64 << 10;
constexpr size_t ADOPT_ROOM = 16 << 10;
constexpr uint32_t ADOPT_SETTING_COUNT = 7;

typedef struct {
    backend_api_t api;
    backend_err_t err;
    backend_t *be;
    backend_module_t *module;
    backend_memory_t *memory;
    unsigned char *block;
    size_t reserved;
} adopt_t;

typedef struct {
    adopt_t *t;
    const char *name;
    int rounds;
    int failed;
} adopt_job_t;

typedef struct {
    backend_t *be;
    backend_module_t *module;
    backend_instance_t *instance;
    backend_func_t *f;
} adopt_side_t;

typedef struct {
    adopt_t *t;
    bool below;
    bool called;
    void *bytes;
    size_t len;
    unsigned long long left;
    adopt_side_t side[2];
    backend_err_t err;
} adopt_margin_t;

void *adopt_read_file(const char *path, size_t *len);

backend_t *adopt_start_backend(const adopt_t *t, int64_t stack,
    backend_err_t *err);

backend_instance_t *adopt_make_instance(adopt_t *t);

int adopt_run(adopt_t *t, backend_instance_t *i, const char *name,
    int32_t *status);

[[gnu::format(printf, 1, 2)]] int adopt_fail(const char *format, ...);

int adopt_open(adopt_t *t);
int adopt_check_write(adopt_t *t, backend_instance_t *one);
int adopt_check_two_stores(adopt_t *t, backend_instance_t *one);
int adopt_check_stray(adopt_t *t, backend_instance_t *one);
int adopt_check_host_function(adopt_t *t);
int adopt_check_timed_calls(adopt_t *t);
int adopt_check_functions(adopt_t *t, backend_instance_t *one);
int adopt_check_refusals(adopt_t *t, backend_instance_t *one);
int adopt_check_edges(adopt_t *t, backend_instance_t *one);
int adopt_check_margin(adopt_t *t, bool below);

int adopt_say_refused(const char *what, bool refused,
    const backend_err_t *err);

#endif
