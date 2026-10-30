/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_BACKENDS_WASM_INSIDE_H
    #define BRIDGE_BACKENDS_WASM_INSIDE_H

    #include <wasm.h>
    #include <wasmtime.h>

    #include "bridge_backend.h"
    #include "os.h"

constexpr uint32_t WASM_PAGE = 65'536;

constexpr uint64_t WASM_FAR = (uint64_t)1 << 62;
constexpr uint64_t WASM_FUEL = UINT64_MAX;
constexpr int64_t WASM_OWN = 1;

constexpr char WASM_NO_MEMORY[] = "out of memory";

enum wasm_engine : uint8_t {
    WASM_METERED = 1,
    WASM_INTERRUPTIBLE = 2
};

constexpr int WASM_ENGINES = 4;

enum wasm_key : uint8_t {
    WASM_KEY_RESERVATION = 0,
    WASM_KEY_GUARD = 1,
    WASM_KEY_STACK = 2,
    WASM_KEY_MARGIN = 3,
    WASM_KEY_WATCHDOG = 4,
    WASM_KEY_LEVEL = 5,
    WASM_KEY_COMPILE = 6
};

constexpr int WASM_SETTINGS = 7;

typedef struct {
    uint64_t reservation;
    uint64_t guard;
    uint64_t wasm_stack;
    uint64_t stack_margin;
    uint64_t watchdog_stack;
    wasmtime_opt_level_t opt_level;
    bool parallel_compilation;
} wasm_settings_t;

struct backend {
    wasm_engine_t *engines[WASM_ENGINES];
    os_thread_t *watchdog;
    os_event_t *wake;
    os_mutex_t *lock;
    uint64_t active;
    uint64_t calls;
    uint64_t until;
    wasm_engine_t *ticking;
    backend_memory_t *adopting;
    wasm_settings_t settings;
    int quit;
};

typedef struct {
    uint64_t active;
    uint64_t until;
    wasm_engine_t *engine;
    int quit;
} wasm_armed_t;

struct backend_memory {
    void *ptr;
    uint64_t len;
    wasmtime_module_t *holder[WASM_ENGINES];
};

struct backend_module {
    backend_t *b;
    enum wasm_engine kind;
    wasmtime_module_t *module;
    wasm_importtype_vec_t imports;
    wasm_exporttype_vec_t exports;
};

struct backend_func {
    backend_instance_t *instance;
    const wasm_name_t *name;
    wasmtime_func_t func;
    backend_func_t *next;
};

struct backend_instance {
    backend_t *b;
    backend_module_t *m;
    wasm_engine_t *engine;
    wasmtime_store_t *store;
    wasmtime_context_t *ctx;
    wasmtime_linker_t *linker;
    wasmtime_instance_t instance;
    int instantiated;
    uint64_t deadline;
    int64_t memory_limit;
    int64_t element_limit;
    int64_t memories;
    backend_func_t *funcs;
};

typedef struct {
    uint8_t *base;
    size_t size;
    size_t reserve;
    size_t guard;
} wasm_own_t;

typedef struct {
    backend_fn_t fn;
    void *ud;
    uint32_t params;
    uint32_t results;
    uint8_t param_kind[16];
    uint8_t result_kind[4];
} wasm_offered_t;

void wasm_drop(wasmtime_error_t *error);

int wasm_fail(backend_err_t *err, const char *what, wasmtime_error_t *error,
    wasm_trap_t *trap);

int wasm_read_settings(wasm_settings_t *s, const backend_setting_t *settings,
    uint32_t count, backend_err_t *err);

wasmtime_error_t *wasm_make_own(const backend_t *b, size_t minimum,
    wasmtime_linear_memory_t *ret);

wasmtime_error_t *wasm_adopt_into(backend_instance_t *i, backend_memory_t *mem,
    wasmtime_extern_t *item, wasm_trap_t **trap);

void wasm_watch(void *arg);

int wasm_start_watch(backend_t *b, backend_err_t *err);

wasm_engine_t *wasm_get_engine(backend_t *b, enum wasm_engine kind);

wasm_offered_t *wasm_make_offered(const backend_entry_t *entry);

void wasm_make_types(wasm_valtype_vec_t *vec, const enum backend_type *kinds,
    uint32_t count);

wasm_trap_t *wasm_call_offered(void *env, wasmtime_caller_t *caller,
    wasmtime_val_raw_t *raw, size_t count);

void wasm_set_limits(backend_instance_t *i);

BACKEND_EXPORT void instance_set_work_limit(backend_instance_t *i,
    uint64_t units);

void wasm_find_func(backend_instance_t *i, backend_func_t *f);

#endif
