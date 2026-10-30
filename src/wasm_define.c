/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "wasm_inside.h"

static wasm_functype_t *make_type(const backend_sig_t *sig)
{
    wasm_valtype_vec_t params;
    wasm_valtype_vec_t results;

    wasm_make_types(&params, sig->params, sig->param_count);
    wasm_make_types(&results, sig->results, sig->result_count);
    return (wasm_functype_new(&params, &results));
}

BACKEND_EXPORT int define_func(backend_instance_t *i,
    const backend_entry_t *entry, backend_err_t *err)
{
    const backend_sig_t *sig = entry->sig;
    wasm_offered_t *h;
    wasm_functype_t *ty;
    wasmtime_error_t *error;

    if (i->instantiated || sig->param_count > 16 || sig->result_count > 4)
        return (wasm_fail(err, "define_func: after the first call of the "
            "module, or too wide", nullptr, nullptr));
    h = wasm_make_offered(entry);
    ty = make_type(sig);
    error = wasmtime_linker_define_func_unchecked(i->linker, entry->ns,
        strlen(entry->ns), entry->field, strlen(entry->field), ty,
        wasm_call_offered, h, free);
    wasm_functype_delete(ty);
    if (error)
        return (wasm_fail(err, "cannot define a function", error, nullptr));
    return (0);
}
