/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wasm_inside.h"

static const wasm_exporttype_t *find_export(const backend_module_t *m,
    const char *name)
{
    size_t len = strlen(name);
    const wasm_name_t *at;
    size_t k = 0;

    while (k < m->exports.size) {
        at = wasm_exporttype_name(m->exports.data[k]);
        if (at->size == len && !memcmp(at->data, name, len))
            return (m->exports.data[k]);
        k++;
    }
    return (nullptr);
}

static bool is_shaped(const wasm_functype_t *ty)
{
    const wasm_valtype_vec_t *results = wasm_functype_results(ty);

    return (!wasm_functype_params(ty)->size && results->size == 1
        && wasm_valtype_kind(results->data[0]) == WASM_I32);
}

static int check_export(const wasm_exporttype_t *export, const char *name,
    backend_err_t *err)
{
    constexpr size_t room = sizeof(err->text);
    const wasm_functype_t *ty = nullptr;

    if (export)
        ty = wasm_externtype_as_functype_const(wasm_exporttype_type(export));
    if (ty && is_shaped(ty))
        return (0);
    if (!err)
        return (1);
    if (ty)
        snprintf(err->text, room, "its export '%.100s' is not () -> i32", name);
    else
        snprintf(err->text, room, "no exported function '%s'", name);
    return (1);
}

void wasm_find_func(backend_instance_t *i, backend_func_t *f)
{
    wasmtime_extern_t item = {};

    wasmtime_instance_export_get(i->ctx, &i->instance, f->name->data,
        f->name->size, &item);
    f->func = item.of.func;
    wasmtime_extern_delete(&item);
}

BACKEND_EXPORT backend_func_t *instance_get_func(backend_instance_t *i,
    const char *name, backend_err_t *err)
{
    const wasm_exporttype_t *export = find_export(i->m, name);
    backend_func_t *f;

    if (check_export(export, name, err))
        return (nullptr);
    f = calloc(1, sizeof(*f));
    if (!f) {
        wasm_fail(err, WASM_NO_MEMORY, nullptr, nullptr);
        return (nullptr);
    }
    f->instance = i;
    f->name = wasm_exporttype_name(export);
    f->next = i->funcs;
    i->funcs = f;
    if (i->instantiated)
        wasm_find_func(i, f);
    return (f);
}
