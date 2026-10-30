/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "adopt.h"

static int run_all(adopt_t *t)
{
    backend_instance_t *one = adopt_make_instance(t);
    int failed = adopt_check_write(t, one) || adopt_check_two_stores(t, one)
        || adopt_check_stray(t, one) || adopt_check_host_function(t)
        || adopt_check_timed_calls(t) || adopt_check_refusals(t, one)
        || adopt_check_edges(t, one) || adopt_check_margin(t, false);

    t->api.instance_destroy(one);
    return (failed);
}

int main(int argc, char **argv)
{
    adopt_t t = {};
    int failed;

    setvbuf(stdout, nullptr, _IONBF, 0);
    if (adopt_open(&t))
        return (1);
    if (argc > 1 && !strcmp(argv[1], "below"))
        failed = adopt_check_margin(&t, true);
    else
        failed = run_all(&t);
    t.api.module_destroy(t.module);
    t.api.memory_release(t.memory);
    t.api.backend_destroy(t.be);
    os_release(t.block, t.reserved);
    return (failed);
}
