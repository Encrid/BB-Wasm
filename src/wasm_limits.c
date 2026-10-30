/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wasm_inside.h"

void wasm_set_limits(backend_instance_t *i)
{
    wasmtime_store_limiter(i->store, i->memory_limit, i->element_limit, -1,
        WASM_OWN, i->memories);
}

BACKEND_EXPORT void instance_set_memory_limit(backend_instance_t *i,
    uint64_t bytes)
{
    i->memory_limit = (int64_t)bytes;
    wasm_set_limits(i);
}

BACKEND_EXPORT void instance_set_element_limit(backend_instance_t *i,
    uint64_t elements)
{
    i->element_limit = (int64_t)elements;
    wasm_set_limits(i);
}
