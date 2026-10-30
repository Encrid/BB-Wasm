/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "cases.h"

static const bridge_setting_t TEST_SETTING[1] = {
    { "cache", BRIDGE_STRING, {}, { .s = "somewhere" } }
};

static const test_edit_t TEST_BACKEND_EDITS[] = {
    { "opt_level=unset", offsetof(host_t, backends[0].setting_count), 4,
        HOST_LEVEL },
    { "reservation=text", offsetof(host_t, settings[HOST_RESERVATION].kind),
        1, BRIDGE_STRING },
    { "reservation=-1", offsetof(host_t, settings[HOST_RESERVATION].v.i), 8,
        -1 },
    { "guard=-1", offsetof(host_t, settings[HOST_GUARD].v.i), 8, -1 },
    { "guard=0", offsetof(host_t, settings[HOST_GUARD].v.i), 8, 0 },
    { "max_wasm_stack=0", offsetof(host_t, settings[HOST_STACK].v.i), 8, 0 },
    { "stack_margin=-1", offsetof(host_t, settings[HOST_MARGIN].v.i), 8, -1 },
    { "stack_margin=0", offsetof(host_t, settings[HOST_MARGIN].v.i), 8, 0 },
    { "watchdog_stack=-1", offsetof(host_t, settings[HOST_WATCHDOG].v.i), 8,
        -1 },
    { "watchdog_stack=huge", offsetof(host_t, settings[HOST_WATCHDOG].v.i),
        8, 1ll << 60 },
    { "opt_level=integer", offsetof(host_t, settings[HOST_LEVEL].kind), 1,
        BRIDGE_INT },
    { "parallel_compilation=on", offsetof(host_t, settings[HOST_COMPILE].v.b),
        1, true },
    { "parallel_compilation=integer",
        offsetof(host_t, settings[HOST_COMPILE].kind), 1, BRIDGE_INT }
};

void test_hand_setting(host_t *h)
{
    h->backends[0].settings = TEST_SETTING;
    h->backends[0].setting_count = 1;
    host_open_only(h);
}

void test_edit_backend(host_t *h)
{
    const test_edit_t *e = test_find_edit(TEST_BACKEND_EDITS, h->arg);

    test_put_key((char *)h + e->at, e->size, e->value);
    if (host_open(h))
        return;
    test_load_small(h, "ok");
    host_close(h);
}

void test_level(host_t *h)
{
    h->settings[HOST_LEVEL].v.s = h->arg;
    if (host_open(h))
        return;
    test_load_small(h, "ok");
    host_close(h);
}

void test_set_and_load(host_t *h)
{
    const char *value = strchr(h->arg, '=') + 1;
    char identity[64];
    bridge_artifact_t a;
    uint32_t k = 0;

    while (strncmp(h->settings[k].key, h->arg, (size_t)(value - 1 - h->arg)))
        k++;
    h->settings[k].v.i = atoll(value);
    if (host_open(h))
        return;
    test_state_small(&a, identity, strchr(value, ' ') + 1);
    a.cfg.memory_limit = BRIDGE_NO_LIMIT;
    host_load(h, &a);
    host_close(h);
}
