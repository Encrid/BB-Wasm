/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "wasm_inside.h"

BACKEND_EXPORT backend_t *backend_create(const backend_setting_t *settings,
    uint32_t count, backend_err_t *err)
{
    backend_t *b = calloc(1, sizeof(*b));

    if (wasm_read_settings(&b->settings, settings, count, err)) {
        free(b);
        return (nullptr);
    }
    b->wake = os_event_create();
    b->lock = os_mutex_create();
    b->watchdog = os_thread_start(wasm_watch, b,
        (size_t)b->settings.watchdog_stack);
    return (b);
}

BACKEND_EXPORT void backend_destroy(backend_t *b)
{
    int k = 0;

    os_mutex_lock(b->lock);
    b->quit = 1;
    os_mutex_unlock(b->lock);
    os_event_set(b->wake);
    os_thread_join(b->watchdog);
    os_event_free(b->wake);
    os_mutex_free(b->lock);
    while (k < WASM_ENGINES) {
        if (b->engines[k])
            wasm_engine_delete(b->engines[k]);
        k++;
    }
    free(b);
}

BACKEND_EXPORT enum backend_capability backend_get_capabilities(
    [[maybe_unused]] backend_t *b)
{
    return (BACKEND_WORK_LIMIT | BACKEND_TIMEOUT | BACKEND_MEMORY_LIMIT
        | BACKEND_ELEMENT_LIMIT | BACKEND_RELOAD | BACKEND_SIDE_BY_SIDE);
}

BACKEND_EXPORT uint32_t backend_get_abi(void)
{
    return (BACKEND_ABI);
}

BACKEND_EXPORT void memory_get_shape(backend_t *b, uint64_t *reservation,
    uint64_t *guard, uint64_t *granule)
{
    *reservation = b->settings.reservation;
    *guard = b->settings.guard;
    *granule = WASM_PAGE;
}
