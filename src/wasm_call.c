/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "wasm_inside.h"

static uint64_t arm(backend_instance_t *i)
{
    backend_t *b = i->b;
    uint64_t nanos = i->deadline;

    i->deadline = 0;
    if (!(i->m->kind & WASM_INTERRUPTIBLE))
        return (0);
    wasmtime_context_set_epoch_deadline(i->ctx, nanos ? 1 : WASM_FAR);
    if (!nanos)
        return (0);
    os_mutex_lock(b->lock);
    b->calls++;
    b->active = b->calls;
    b->until = os_get_now() + nanos;
    b->ticking = i->engine;
    os_mutex_unlock(b->lock);
    os_event_set(b->wake);
    return (nanos);
}

static void disarm(backend_t *b)
{
    os_mutex_lock(b->lock);
    b->active = 0;
    os_mutex_unlock(b->lock);
    os_event_set(b->wake);
}

static int instantiate(backend_instance_t *i, backend_err_t *err)
{
    backend_func_t *f = i->funcs;
    wasm_trap_t *trap = nullptr;
    wasmtime_error_t *error;

    if (i->instantiated)
        return (0);
    error = wasmtime_linker_instantiate(i->linker, i->ctx, i->m->module,
        &i->instance, &trap);
    if (error || trap)
        return (wasm_fail(err, "made no instance", error, trap));
    i->instantiated = 1;
    while (f) {
        wasm_find_func(i, f);
        f = f->next;
    }
    return (0);
}

static int run(const backend_func_t *f, int32_t *status, backend_err_t *err)
{
    wasmtime_val_t result;
    wasm_trap_t *trap = nullptr;
    wasmtime_error_t *error;

    error = wasmtime_func_call(f->instance->ctx, &f->func, nullptr, 0, &result,
        1, &trap);
    if (error || trap)
        return (wasm_fail(err, "trapped", error, trap));
    *status = result.of.i32;
    return (0);
}

BACKEND_EXPORT int call(backend_func_t *f, int32_t *status, backend_err_t *err)
{
    backend_instance_t *i = f->instance;
    const wasm_settings_t *s = &i->b->settings;
    uint64_t need = s->wasm_stack + s->stack_margin;
    uint64_t left = os_thread_stack_left();
    uint64_t nanos;
    int failed;

    if (left < need) {
        i->deadline = 0;
        snprintf(err->text, sizeof(err->text), "did not run: the thread has "
            "%llu bytes of stack left, and max_wasm_stack and stack_margin "
            "need %llu", (unsigned long long)left, (unsigned long long)need);
        return (1);
    }
    nanos = arm(i);
    failed = instantiate(i, err) || run(f, status, err);
    if (nanos)
        disarm(i->b);
    instance_set_work_limit(i, WASM_FUEL);
    return (failed);
}
