/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "wasm_inside.h"

static uint8_t *get_own(void *env, size_t *size, size_t *capacity)
{
    wasm_own_t *m = env;

    *size = m->size;
    *capacity = m->reserve;
    return (m->base);
}

static wasmtime_error_t *grow_own(void *env, size_t new_size)
{
    wasm_own_t *m = env;

    if (new_size > m->reserve || os_commit(m->base, new_size))
        return (wasmtime_error_new("out of address space for a module's own "
            "memory"));
    m->size = new_size;
    return (nullptr);
}

static void free_own(void *env)
{
    wasm_own_t *m = env;

    os_release(m->base, m->reserve + m->guard);
    free(m);
}

wasmtime_error_t *wasm_make_own(const backend_t *b, size_t minimum,
    wasmtime_linear_memory_t *ret)
{
    wasm_own_t *m = malloc(sizeof(*m));

    m->reserve = (size_t)b->settings.reservation;
    m->guard = (size_t)b->settings.guard;
    m->size = minimum;
    m->base = nullptr;
    if (minimum <= m->reserve)
        m->base = os_reserve(m->reserve + m->guard, minimum);
    if (!m->base) {
        free(m);
        return (wasmtime_error_new("no room for a module's own memory: it "
            "starts past the reservation, or the address space is full"));
    }
    ret->env = m;
    ret->get_memory = get_own;
    ret->grow_memory = grow_own;
    ret->finalizer = free_own;
    return (nullptr);
}
