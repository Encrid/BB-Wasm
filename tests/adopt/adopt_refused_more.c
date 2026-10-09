/*
** Copyright (C) 2026 BB-Wasm Owner
**
** This file is part of BB-Wasm.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/adopt/adopt_refused_more.c -- what a backend refuses of whoever
** calls it: a memory handed to the wasm backend twice under one name, and
** a memory given twice under one name.
*/

#include "adopt.h"

static int refuse_memory(adopt_t *t)
{
    backend_err_t err = {};
    backend_memory_import_t imports[2] = {{ "bridge", "shared", t->memory },
        { "bridge", "shared", t->memory }};
    backend_instance_t *i = t->api.instance_create(t->module, imports, 2,
        &err);

    if (i)
        t->api.instance_destroy(i);
    return (adopt_say_refused("a memory given twice under one name", !i,
        &err));
}

/*
** adopt_check_refusals
**
** Each thing a backend refuses of whoever calls it is refused, and says
** why.
*/
int adopt_check_refusals(adopt_t *t, backend_instance_t *one)
{
    return (adopt_check_functions(t, one) || refuse_memory(t));
}
