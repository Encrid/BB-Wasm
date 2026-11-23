/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "cases.h"

static void test_load_timed(host_t *h, const char *name, int64_t timeout)
{
    char identity[64];
    bridge_artifact_t a;

    test_state_small(&a, identity, name);
    a.cfg.work_limit = BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = timeout;
    host_load(h, &a);
}

void test_time_after_a_late_deadline(host_t *h)
{
    if (host_open(h))
        return;
    test_load_timed(h, "spinfirst", 1);
    test_load_timed(h, "spinlong", strtoll(h->arg, nullptr, 10) * HOST_MS);
    host_close(h);
}

void test_load_spinning(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_small(&a, identity, h->arg);
    a.cfg.init_timeout = TEST_SPIN_DEADLINE;
    host_load(h, &a);
    host_close(h);
}

void test_set_and_spin(host_t *h)
{
    test_set_and_load_timed(h, TEST_SPIN_DEADLINE);
}
